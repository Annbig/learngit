/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "lc_ul_reassembly.h"
#include <math.h>
#include <string.h>

//#define SN_LENGTH          3                         // LC header中SN比特数——用3进行测试
#define UM_Window_Size     6                           // 窗口大小根据SN值设置
#define UM_BUFFER_LEN      12
#define mode22headerSize   7                           // mode 22 headerSize == 7

extern GtList lcEntityList;                // LC实体对应的链表
extern GtList TNodeList;                   // T节点对应的链表 


/*---------------------------------下述函数为定时器相关代码-----------------------------------------*/
void checkUmRecvTimer(GtLcEntity* lcEntity);
void startUmTimer(GtLcEntity* lcEntity);
static void recvWindowTimerCallback(TimerHandle_t recvWindowTimerHandle);
static TimerHandle_t recvWindowTimer = NULL;

/*---------------------------------------------------------------------------------------------*/

void getLcPdu() {
    // 遍历LC实体的链表，根据对应关系找到上行逻辑信道的上行数据缓冲区
    GtListNode* curEntity = &lcEntityList.node;
    gt_u32 entityCount = lcEntityList.count;

    for (gt_u32 i = 0; i < entityCount; i++)
    {
        curEntity = curEntity->next;
        gt_u8  lcId = ((GtLcEntity*)curEntity)->lcId;
        gt_u32 TNodeID = ((GtLcEntity*)curEntity)->TNodeID;

        GtListNode* curTnode = &TNodeList.node;
        gt_u32 TnodeCount = TNodeList.count;

        for (gt_u32 j = 0; j < TnodeCount; j++)
        {
            curTnode = curTnode->next;
            if (((TNode*)curTnode)->id == TNodeID) break;
        }
        if (((TNode*)curTnode)->id != TNodeID) break;//没找到
        //找到每一个lc实体对应的逻辑信道，然后遍历该逻辑信道中的所有pdu，对于每一个pdu，做？操作
        GtListNode* curLcUlPdu = &(((TNode*)curTnode)->ulLogicChannel[lcId].pduUlBufferList.node);
        gt_u32 pduCount = ((TNode*)curTnode)->ulLogicChannel[lcId].pduUlBufferList.count;

        for (gt_u32 k = 0; k < pduCount; k++)
        {
            curLcUlPdu = curLcUlPdu->next;

            //printf("\n测试getLcPdu()\n");

            //gt_u8  SI = 0; 
            //gt_u32 SN = 0; 
            //gt_u16 SO = 0;

            //gt_u8* pointer = ((GtLcPduBufferNode *)curLcUlPdu)->data;
            //gt_u8 ch = *pointer;

            //// 取ch前两位即为SI
            //gt_u8 n0, n1;
            //n0 = (ch & 0x01) == 0x01 ? 1 : 0;
            //n1 = (ch & 0x02) == 0x02 ? 1 : 0;
            //// printf("\nn0 = %d, n1 = %d\n", n0, n1);
            //SI = (n1 * 2 + n0);

            //// SI包括第一个字节的后两位与第二个字节和第三个字节
            //gt_u8 n6, n7;
            //n6 = (ch & 0x40) == 0x40 ? 1 : 0;
            //n7 = (ch & 0x80) == 0x80 ? 1 : 0;
            //SN = ((n7 << 1) + n6) + (*(pointer + 1) << 2) + (*(pointer + 2) << 10);

            //// SO包括第六个字节和第七个字节
            //SO = *(pointer + 5) + (*(pointer + 6) << 8);
            //printf("SI = %d, SN = %d, SO = %d\n", SI, SN, SO);
            //printf("data = [%c]\n", *(((gt_u8*)((GtLcPduBufferNode*)curLcUlPdu)->data) + 7));

            // 每读取一个PDU就调整一次窗口
            umAssembly((GtLcEntity*)curEntity, (GtLcPduBufferNode*)curLcUlPdu);

            // 检查对应的定时器
            checkUmRecvTimer((GtLcEntity*)curEntity);

            // 释放所有PDU的操作谁来做？
        }
    }
}
void parseLcHeaderMode11() {

}

void parseLcHeaderMode12() {

}

void parseLcHeaderMode21() {

}
//下面这个函数负责解析22模式下的lc头
void parseLcHeaderMode22(gt_u8* SI, gt_u32* SN, gt_u16* SO, GtLcPduBufferNode* lcUlPdu) {
    gt_u8* pointer = lcUlPdu->data;
    gt_u8 ch = *pointer;

    // 取ch前两位即为SI
    gt_u8 n0, n1;
    n0 = (ch & 0x01) == 0x01 ? 1 : 0;
    n1 = (ch & 0x02) == 0x02 ? 1 : 0;
    *SI = (n1 << 1) + n0;

    // SI包括第一个字节的后两位与第二个字节和第三个字节
    gt_u8 n6, n7;
    n6 = (ch & 0x40) == 0x40 ? 1 : 0;
    n7 = (ch & 0x80) == 0x80 ? 1 : 0;
    *SN = ((n7 << 1) + n6) + (*(pointer + 1) << 2) + (*(pointer + 2) << 10);

    // SO包括第六个字节和第七个字节
    *SO = *(pointer + 5) + (*(pointer + 6) << 8);
}

void parseLcHeader(gt_u8* SI, gt_u32* SN, gt_u16* SO, GtLcEntity* lcEntity, GtLcPduBufferNode* lcUlPdu) {
    gt_u8 lcMode = lcEntity->lcMode;
    switch (lcMode) {
    case 0:
        parseLcHeaderMode11();
        break;
    case 1:
        parseLcHeaderMode12();
        break;
    case 2:
        parseLcHeaderMode21();
        break;
    case 3:
        parseLcHeaderMode22(SI, SN, SO, lcUlPdu);
        break;
    default:break;
    }
}

/*
 * 函数功能：计算接收窗口的范围
 * 参数说明：
 *  （1）scale[]：结果数组，长度为4，设置长度为4是为了应对翻转的情况，具体定义为[下边界1，上边界1，下边界2，上边界2]，如果窗口不存在翻转的情况的话，下边界1 == 上边界1 == 0
 *  （2）(RX_Next_Highest – UM_Window_Size) <= x < RX_Next_Highest表示的就是整个窗口的范围
 * 返回值说明：现在窗口的范围就是[scale[0], scale[1]) + [scale[2], scale[3])//这个地方还是没懂？但感觉也不用懂
 * 以下为代码测试结果：//可以理解为rx-next-highest是指最后一个数字的前一个窗口，然后所有窗口只有0~7
 * 4 8 0 0 对应的窗口为4，5，6，7  scale：0 0 4 7
 * 5 8 0 1 对应的窗口为5，6，7，0         0 0 5 7
 * 6 8 0 2 对应的窗口为6，7，0，1         0 1 6 7
 * 7 8 0 3 对应的窗口为7，0，1，2         0 2 7 7
 * 0 0 0 4 对应的窗口为0，1，2，3         0 3 0 0
 * 0 0 1 5 对应的窗口为1，2，3，4         1 4 0 0
 * 0 0 2 6 对应的窗口为2，3，4，5         2 5 0 0
 * 0 0 3 7 对应的窗口为3，4，5，6         3 6 0 0
 */
void calculateUmRxWindowScale(gt_u32 scale[], gt_u32 RX_Next_Highest) {//就是更新接收窗口的函数
    if (RX_Next_Highest < UM_Window_Size)
    {
        scale[0] = RX_Next_Highest + UM_BUFFER_LEN - UM_Window_Size;
        scale[1] = UM_BUFFER_LEN;
        scale[2] = 0;
        scale[3] = RX_Next_Highest;
    }
    else {
        scale[0] = 0;
        scale[1] = 0;
        scale[2] = RX_Next_Highest - UM_Window_Size;
        scale[3] = RX_Next_Highest;
    }
}
//释放接收窗口的pdu列表（在提交之后）
void releaseUmRecvWinElement(umRecvWindowElement* ulPduArray, gt_u32 begin, gt_u32 end) {

    while (begin != end) {
        umRecvWindowElement* curWinElement = &ulPduArray[begin];
        GtListNode* curNode = &(curWinElement->node);
        for (gt_u32 i = 0; i < curWinElement->count; i++) {
            curNode = curNode->next;
        }
        // 从后向前释放
        for (gt_u32 i = 0; i < curWinElement->count; i++) {
            curNode = curNode->pre;
            vPortFree(curNode->next);
            curNode->next = NULL;
        }
        // 修改数组元素对应的属性
        curWinElement->count = 0;
        curWinElement->totalLen = 0;
        curWinElement->curLen = 0;

        // 此处不需要修改curWinElement->finish

        begin = (begin + 1) % UM_BUFFER_LEN;
    }
}

/*
 * 函数功能：判断index是否在窗口中
 */
gt_bool isInUmRecvWindow(gt_u32 scale[], gt_u32 index)
{
    if ((index >= scale[0] && index < scale[1]) || (index >= scale[2] && index < scale[3])) {
        return true;
    }
    return false;
}

/*
 * 函数功能：向UM接收窗口中挂载数据（仅仅是挂载一个pdu）
 */
void pushDataInUmRecvWindow(GtLcEntity* lcEntity, gt_u8  SI, gt_u32 SN, gt_u16 SO, GtLcPduBufferNode* lcUlPdu) {
    // 新建数组元素链表节点 
    // lcUlPdu->len - mode22headerSize == dataLen
    umRecvWinEleListNode* newRxWinEleListNode = pvPortMalloc(sizeof(umRecvWinEleListNode) + lcUlPdu->len - mode22headerSize);
    newRxWinEleListNode->SO = SO;
    newRxWinEleListNode->len = lcUlPdu->len - mode22headerSize;
    memcpy(newRxWinEleListNode->data, (gt_u8*)lcUlPdu->data + mode22headerSize, newRxWinEleListNode->len);

    printf("\nnewRxWinEleListNode->data = [%c]\n", *newRxWinEleListNode->data);
    // 更新数组元素 也就是将新建的接收窗口的元素节点放入窗口元素的链表末尾
    umRecvWindowElement* curWinElement = &(lcEntity->umRxWindow.ulPduArray[SN]);
    GtListNode* curNode = &curWinElement->node;
    for (gt_u32 i = 0; i < curWinElement->count; i++) {
        curNode = curNode->next;
    }
    curNode->next = &newRxWinEleListNode->node;

    newRxWinEleListNode->node.pre = curNode;
    newRxWinEleListNode->node.next = NULL;
    curWinElement->count++;

    curWinElement->curLen += newRxWinEleListNode->len;

    // 更新totalLen的条件是收到SI == 3的SDU分段
    if (SI == 0 || SI == 3) curWinElement->totalLen = newRxWinEleListNode->len + SO;

    // 判断该SDU是否重组完成，SI == 0表示该PDU包含一个完整的SDU，可以直接向上交付
    if (curWinElement->totalLen == curWinElement->curLen) {
        // 重组SDU递交给上层
        GtListNode* curUlbufferNode = &lcEntity->ulBufferList;
        for (gt_u32 j = 0; j < lcEntity->ulBufferList.count; j++) {
            curUlbufferNode = curUlbufferNode->next;
        }
        GtLcEntitybufferListNode* newUlbufferNode = pvPortMalloc(sizeof(GtLcEntitybufferListNode) + curWinElement->totalLen);
        curUlbufferNode->next = &newUlbufferNode->node;
        newUlbufferNode->node.pre = curUlbufferNode;
        newUlbufferNode->node.next = NULL;
        newUlbufferNode->len = curWinElement->totalLen;

        curNode = &curWinElement->node;
        for (gt_u32 k = 0; k < curWinElement->count; k++) {
            curNode = curNode->next;
            memcpy((gt_u8*)newUlbufferNode->data + ((umRecvWinEleListNode*)curNode)->SO,
                ((umRecvWinEleListNode*)curNode)->data,
                ((umRecvWinEleListNode*)curNode)->len);
        }
        printf("\nnewUlbufferNode->data = [%s]\n", newUlbufferNode->data);

        // 释放对应的空间，注意释放顺序为从后向前释放
        for (gt_u32 m = 0; m < curWinElement->count; m++) {
            curNode = curNode->pre;
            vPortFree(curNode->next);    // 对应的是malloc(umRecvWinEleListNode)
            curNode->next = NULL;
        }

        // 修改数组元素对应的属性 （指的是窗口数组元素）
        curWinElement->count = 0;
        curWinElement->totalLen = 0;
        curWinElement->curLen = 0;
        curWinElement->finish = 1;//如果已经重组完成了就可以修改finish为1
        //其中finish象征着当前窗口元素是否已经是一个完整的SDU，如果是一个完整的SDU，那么它就可以向上交付了

        gt_u32 RX_Next_Reassembly = lcEntity->umRxWindow.RX_Next_Reassembly;
        while (lcEntity->umRxWindow.ulPduArray[RX_Next_Reassembly].finish == 1) {
            RX_Next_Reassembly = (RX_Next_Reassembly + 1) % UM_BUFFER_LEN;
        }
        lcEntity->umRxWindow.RX_Next_Reassembly = RX_Next_Reassembly;
    }
    else {// 未重组完成，将数组元素中的finish字段更新
        curWinElement->finish = 0;
    }
}
//两个pdu是否在同一个接收窗口范围内（有何意义？）
gt_bool isSameSide(gt_u32 scale[], gt_u32 firstIndex, gt_u32 secondIndex) {
    return ((firstIndex >= scale[0] && firstIndex < scale[1]) && (secondIndex >= scale[0] && secondIndex < scale[1])) ||
        ((firstIndex >= scale[2] && firstIndex < scale[3]) && (secondIndex >= scale[2] && secondIndex < scale[3]));
}
//判断当前SN的pdu是否应该被放入接收窗口
gt_bool UmPduShouldBePutInWindow(gt_u32 scale[], gt_u32 SN, gt_u32 RX_Next_Reassembly) {
    return ((SN >= RX_Next_Reassembly && isSameSide(scale, SN, RX_Next_Reassembly)) ||
        ((RX_Next_Reassembly >= scale[0] && RX_Next_Reassembly < scale[1]) && (SN >= scale[2] && SN < scale[3])));
}
//上行重组功能 参数是某一个lc实体和它对应的pdu链表的头结点
void umAssembly(GtLcEntity* lcEntity, GtLcPduBufferNode* lcUlPdu) {
    // 解析PDU Header，获取SI、SN和SO
    gt_u8  SI = 0;
    gt_u32 SN = 0;
    gt_u16 SO = 0;
    parseLcHeader(&SI, &SN, &SO, lcEntity, lcUlPdu);

    // 判断SN是否在窗口中
    // 获取当前窗口范围
    gt_u32 scale[4] = { 0 };
    calculateUmRxWindowScale(scale, lcEntity->umRxWindow.RX_Next_Highest);//更新接收窗口的范围
    printf("\n窗口：[%d, %d], [%d, %d]\n", scale[0], scale[1], scale[2], scale[3]);

    // 判断SN和窗口的关系
    if (isInUmRecvWindow(scale, SN)) { // SN在窗口中
        // 判断SN是否应该进入缓存
        if (UmPduShouldBePutInWindow(scale, SN, lcEntity->umRxWindow.RX_Next_Reassembly)) {// 应该进入缓存
            pushDataInUmRecvWindow(lcEntity, SI, SN, SO, lcUlPdu);
        }// 没有进入缓存，直接丢弃，不再处理
    }
    else {// SN不在窗口中，对于不在窗口中的情况，一律假定为后出窗
       // 更新RX_Next_Highest = SN + 1，注意取模运算
        gt_u32 RX_Next_Highest = (SN + 1) % UM_BUFFER_LEN;
        //计算新的窗口
        gt_u32 newScale[4] = { 0 };
        calculateUmRxWindowScale(newScale, RX_Next_Highest);
        // 释放前出窗的UMD PDU， 这里主要要区分begin和end的值
        gt_u32 begin = 0; // begin 对应的是scale
        gt_u32 end = 0;   // end   对应的是newscale
        if (scale[0] == 0 && scale[1] == 0) {
            begin = scale[2];
        }
        else {
            begin = scale[0];
        }
        if (newScale[0] == 0 && newScale[1] == 0) {
            end = newScale[2];
        }
        else {
            end = newScale[0];
        }
        releaseUmRecvWinElement(lcEntity->umRxWindow.ulPduArray, begin, end);
        lcEntity->umRxWindow.RX_Next_Highest = RX_Next_Highest;
        // 如果此时也导致了RX_Next_Reassembly前出窗，则将RX_Next_Reassembly设置成重组窗口的下边沿
        // 前出窗=>不在窗口内
        if (!isInUmRecvWindow(newScale, lcEntity->umRxWindow.RX_Next_Reassembly)) {
            lcEntity->umRxWindow.RX_Next_Reassembly = end;
        }

        pushDataInUmRecvWindow(lcEntity, SI, SN, SO, lcUlPdu);

    }
}



/*
 * 函数功能：判断SN = RX_Next_Reassembly的UMD PDU序列中是否存在SO空洞
 */
gt_bool isPduSeries(gt_u32 RX_Next_Reassembly, umRecvWindowElement* ulPduArray) {

    return !(ulPduArray[RX_Next_Reassembly].totalLen != ulPduArray[RX_Next_Reassembly].curLen &&
        ulPduArray[RX_Next_Reassembly].totalLen != 0);
}

/*
 * 函数功能：判断是否满足UM接收窗口定时器的触发条件
 */
gt_bool checkUmTimerTriggerCon(GtLcEntity* lcEntity) {
    // 检查启动条件
    // (1)RX_Next_Highest > RX_Next_Reassembly + 1
    // (2)RX_Next_Highest = RX_Next_Reassembly + 1，并且在SN == RX_Next_Reassembly的UMD PDU序列中至少有一个中间分段没有收到，即，SO中间存在空洞
    gt_u32 RX_Next_Highest = lcEntity->umRxWindow.RX_Next_Highest;
    gt_u32 RX_Next_Reassembly = lcEntity->umRxWindow.RX_Next_Reassembly;
    return (RX_Next_Highest > RX_Next_Reassembly + 1) ||
        ((RX_Next_Highest == RX_Next_Reassembly + 1) && isPduSeries(RX_Next_Reassembly, lcEntity->umRxWindow.ulPduArray));
}

/*
 * 函数功能： 返回第一个SN >= RX_Timer_Trigger的未完全接收的UMD PDU的index
 */
gt_u32 firstIncompleteIndex(gt_u32 index, umRecvWindowElement* ulPduArray) {
    while (ulPduArray[index].finish)
    {
        index = (index + 1) % UM_BUFFER_LEN;
    }
    return index;
}
//开启定时器
void startUmTimer(GtLcEntity* lcEntity) {
    // 设置RX_Timer_Trigger = RX_Next_Highest(说明RX_Timer_Trigger之前存在未接收的分段)
    lcEntity->umRxWindow.RX_Timer_Trigger = lcEntity->umRxWindow.RX_Next_Highest;
    lcEntity->umRxWindow.isTrigger = 1;
    /*lcEntity->umRxWindow.t_Reassembly = Time + lcEntity->trr;*/
    // 创建定时器并指定对应的回调函数
    recvWindowTimer = xTimerCreate("recvWindowTimer",		           /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
        pdMS_TO_TICKS(lcEntity->trr),	   /* The period of the software timer in ticks. */
        pdFALSE,			               /* xAutoReload is set to pdFALSE, so this is a one-shot timer. */
        (void*)lcEntity->lcEntityID,	   /* The timer's ID  */
        recvWindowTimerCallback);        /* The function executed when the timer expires. */
    xTimerStart(recvWindowTimer, 0); /* The scheduler has not started so use a block time of 0. */
}

/*
 * 函数功能： UM接收窗口定时器超时的回调函数
 * 问题：回调函数一定要定义成静态函数吗？
 */
static void recvWindowTimerCallback(TimerHandle_t recvWindowTimerHandle) {
    // 根据timer ID区分调用回调函数的LC实体
    printf("\n接收窗口定时器超时，超时函数被调用\n");
    gt_u8 lcEntityId = (gt_u8)pvTimerGetTimerID(recvWindowTimerHandle);//w获取计时器对应的lc的实体id
    GtListNode* curEntityNode = &lcEntityList.node;
    gt_u32 entityCount = lcEntityList.count;//w获取实体的数目
    for (gt_u32 i = 0; i < entityCount; i++) {//w遍历lc实体的链表，找到对应于计时器的lc的实体id
        curEntityNode = curEntityNode->next;
        if (((GtLcEntity*)curEntityNode)->lcEntityID == lcEntityId) break;
    }
    if (((GtLcEntity*)curEntityNode)->lcEntityID != lcEntityId) return; // curEntityNode指向UM接收窗口定时器超时的LC实体
    GtLcEntity* lcEntity = (GtLcEntity*)curEntityNode;

    // 以下内容为超时处理    w对应于重组
    // 1. 更新窗口
    //（1）设置RX_Next_Reassembly为第一个SN >= RX_Timer_Trigger的未完全接收的UMD PDU的SN
    //（2）丢弃更新前的RX_Next_Reassembly和更新后的RX_Next_Reassembly之间的UMD分段
    gt_u32 old_RX_Next_Reassembly = lcEntity->umRxWindow.RX_Next_Reassembly;
    lcEntity->umRxWindow.RX_Next_Reassembly = firstIncompleteIndex(lcEntity->umRxWindow.RX_Timer_Trigger, lcEntity->umRxWindow.ulPduArray);
    releaseUmRecvWinElement(lcEntity->umRxWindow.ulPduArray, old_RX_Next_Reassembly, lcEntity->umRxWindow.RX_Next_Reassembly);

    // 复位定时器相关的属性
    lcEntity->umRxWindow.isTrigger = 0;
    lcEntity->umRxWindow.RX_Timer_Trigger = 0;


    // 检查是否满足定时器启动条件
    if (checkUmTimerTriggerCon(lcEntity)) {
        // 触发定时器
        startUmTimer(lcEntity);
    }
}



gt_u32 getDistanceToRX_Next_Highest(gt_u32 RX_Next_Highest, gt_u32 SN) {
    if (RX_Next_Highest >= SN) {
        return RX_Next_Highest - SN;
    }
    else
    {
        return RX_Next_Highest + UM_BUFFER_LEN - SN;
    }
}


//检查是否满足定时器启动条件
void checkUmRecvTimer(GtLcEntity* lcEntity) {
    if (lcEntity->umRxWindow.isTrigger) {
        // 检查是否可以复位定时器
        //（1）RX_Timer_Trigger <= RX_Next_Reassembly
        //（2）RX_Timer_Trigger出窗并且RX_Timer_Trigger!= RX_Next_Highest
        //（3）RX_Next_Highest = RX_Next_Reassembly + 1，并且SN = RX_Next_Reassembly的UMD PDU序列中不存在SO空洞
        gt_u32 RX_Timer_Trigger = lcEntity->umRxWindow.RX_Timer_Trigger;
        gt_u32 RX_Next_Reassembly = lcEntity->umRxWindow.RX_Next_Reassembly;
        gt_u32 RX_Next_Highest = lcEntity->umRxWindow.RX_Next_Highest;
        gt_u32 scale[4] = { 0 };
        calculateUmRxWindowScale(scale, RX_Next_Highest);
        if ((getDistanceToRX_Next_Highest(RX_Next_Highest, RX_Next_Reassembly) <= getDistanceToRX_Next_Highest(RX_Next_Highest, RX_Timer_Trigger)) ||   
            (!isInUmRecvWindow(scale, RX_Timer_Trigger) && RX_Timer_Trigger != RX_Next_Highest) ||        // 对应（2） 
            (RX_Next_Highest == RX_Next_Reassembly + 1 && isPduSeries(RX_Next_Reassembly, lcEntity->umRxWindow.ulPduArray))) {      // 对应（3）
            
            // 停止当前LC实体的计时器
            lcEntity->umRxWindow.RX_Timer_Trigger = 0;
            lcEntity->umRxWindow.isTrigger = 0;
            xTimerStop(recvWindowTimer, 0);
        }
    }
    else {
        // 检查是否满足定时器启动条件
        if (checkUmTimerTriggerCon(lcEntity)) {
            // 触发定时器
            startUmTimer(lcEntity);
        }
    }
}


