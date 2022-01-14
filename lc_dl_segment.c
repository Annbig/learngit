#include "lc_dl_segment.h"
#include "lc_msg_if.h"        // msg的定义
#include "lc_entity_if.h"     // LC实体定义        
#include "logicChannel_if.h"  // 逻辑信道定义
#include "lc_header_if.h"     // LC header定义

// 以下两个头文件可以用安全函数库和DEBUG库代替
#include "string.h"           // 引入memcpy()和memmove()，注意memcpy()的使用可能会造成覆盖
#include "math.h"             // 引入pow()
#include "stdio.h"            // 引入printf，输出错误信息

// 打桩
#include "test_TNode.h"

// LC header size
#define mode22HeaderSize  7   // 模式2-2的头部长度为7字节

// SN BIT
#define SN_LENGTH 18          // SN长度为18位

extern GtList lcEntityList;   // LC实体链表
extern GtList TNodeList;      // T节点链表 

/*
    函数功能：释放remLen == 0的sduBufferListNode
*/
void freeSduBufferListNode() {
    gt_u32 count = TNodeList.count;
    GtListNode* curNode = &TNodeList.node;
    for (gt_u32 i = 0; i < count; i++) {
        curNode = curNode->next;
        for (gt_u8 LCID = 0; LCID < G_LCID_MAX; LCID++)
        {
            GtListNode* curSduHeader = &(((TNode*)curNode)->dlLogicChannel[LCID].sduDlBuffer.sduDlBufferList.node);
            gt_u32      sduCount = ((TNode*)curNode)->dlLogicChannel[LCID].sduDlBuffer.sduDlBufferList.count;
            while (curSduHeader->next != NULL && ((GtLcSduDlBufferNode*)curSduHeader->next)->remLen == 0) {
                GtListNode* tmp = curSduHeader->next;
                if (curSduHeader->next->next != NULL) curSduHeader->next->next->pre = curSduHeader;
                curSduHeader->next = curSduHeader->next->next;
                free(tmp);
                sduCount--;
            }
            ((TNode*)curNode)->dlLogicChannel[LCID].sduDlBuffer.sduDlBufferList.count = sduCount;
        }
    }
}

void GtBsLcDataRecv(gt_void* msg, gt_u16 len)
{
    // 特殊处理LC链表为空的情况
    if (lcEntityList.count == 0) {
        printf("\nlcEntityList is empty\n");
        return;
    }
    
    // 解析消息
    GtBsLcDataDlReqMsg* bsDlTxMsg = (GtBsLcDataDlReqMsg*)msg;

    // TCID => LCID，暂时用LC entity ID代替
    gt_u8 entityId = bsDlTxMsg->lcEntityId;

    // 将数据存入LC实体的下行数据缓冲区
    // 遍历LC链表找到对应的实体
    GtListNode* curLcEntity = &lcEntityList.node;
    for (gt_u32 i = 0; i < lcEntityList.count; i++)
    {
        curLcEntity = curLcEntity->next;
        if (((GtLcEntity*)curLcEntity)->lcEntityID == entityId) break;
    }
    if (((GtLcEntity*)curLcEntity)->lcEntityID != entityId) {
        printf("\ncannot find this entity in lcEntityList\n");
        return; // 没有找到对应的LC实体，直接结束函数
    }  

    // 将msg中的数据复制到LC实体的下行数据缓冲区中
    GtListNode* curBufferNode = &(((GtLcEntity*)curLcEntity)->dlBufferList.node);
    gt_u32        bufferCount = ((GtLcEntity*)curLcEntity)->dlBufferList.count;
    for (gt_u32 i = 0; i < bufferCount; i++)
    {
        curBufferNode = curBufferNode->next;//buffer是一个链表，因此需要找到最后一个节点在后面添加上msg
    }

    GtLcEntitybufferListNode* newBuffferNode = (GtLcEntitybufferListNode*)malloc(sizeof(GtLcEntitybufferListNode) + bsDlTxMsg->dataLen);
    //下面是一个在链表尾端插入节点的操作
    curBufferNode->next = &newBuffferNode->node;
    newBuffferNode->node.pre = curBufferNode;
    newBuffferNode->node.next = NULL;
    ((GtLcEntity*)curLcEntity)->dlBufferList.count++;

    memcpy(newBuffferNode->data, bsDlTxMsg->data, bsDlTxMsg->dataLen);
    newBuffferNode->len = bsDlTxMsg->dataLen;

    //// 释放msg所占的堆的空间，将对应的指针置空
    //// 在这里释放考虑的是消息队列处理的实时性比较差，应当在确保接收到消息后在释放
    //free(bsDlTxMsg);
    //bsDlTxMsg = NULL;
    //msg = NULL;

    // 调用函数GtLcLogicChannelDataPush()将LC SDU交付给T节点的逻辑信道
    GtLcLogicChannelDataPush(curLcEntity, newBuffferNode);
}
//将参数的buffer数据放入lc实体对应的逻辑信道中
void GtLcLogicChannelDataPush(GtLcEntity* lcEntity, GtLcEntitybufferListNode* bufferNode) {
    // 这个函数需要加锁，SN值的修改过程应该具有原子性，加锁对应的API?
    // 获取LC实体对应的TNode ID和LCID
    gt_u32 TNodeID = lcEntity->TNodeID;
    gt_u8  lcId = lcEntity->lcId;

    // 将bufferNode中的数据复制进逻辑信道 
    GtListNode* curTNode = &TNodeList.node;
    for (gt_u32 i = 0; i < TNodeList.count; i++)
    {
        curTNode = curTNode->next;
        if (((TNode*)curTNode)->id == TNodeID) break;//找到对应的T节点
    }
    if (((TNode*)curTNode)->id != TNodeID) return 1; // 没有找到对应的T节点，直接结束函数

    GtLogicChannel* logicChannel = &(((TNode*)curTNode)->dlLogicChannel[lcId]);//获取对应的逻辑信道

    GtListNode* curSduDlBufferNode = &(logicChannel->sduDlBuffer.sduDlBufferList.node);//获取逻辑信道对应的sdu Buffer的头结点
    for (gt_u32 i = 0; i < logicChannel->sduDlBuffer.sduDlBufferList.count; i++) {
        curSduDlBufferNode = curSduDlBufferNode->next;//遍历到sdu buffer的最后一个节点
    }

    GtLcSduDlBufferNode* newSduDlBufferNode = (GtLcSduDlBufferNode*)malloc(sizeof(GtLcSduDlBufferNode) + bufferNode->len);
    //对这最后一个节点后面执行插入操作
    curSduDlBufferNode->next = &newSduDlBufferNode->node;
    newSduDlBufferNode->node.pre = curSduDlBufferNode;
    newSduDlBufferNode->node.next = NULL;
    logicChannel->sduDlBuffer.sduDlBufferList.count++;
    //对节点的内容进行丰富
    memcpy(newSduDlBufferNode->data, bufferNode->data, bufferNode->len);
    newSduDlBufferNode->remLen = bufferNode->len;
    newSduDlBufferNode->SO = 0;
    newSduDlBufferNode->lcMode = lcEntity->lcMode;   // 指示该SDU涉及的传输模式

    logicChannel->sduDlBuffer.size += newSduDlBufferNode->remLen;
    //上述操作是一个接收上层PDU（本层SDU）的第一步操作，响应的，需要将一些标志性的数值改变，例如lc实体的SN数值
    // 更新SduBufferNode的SN值
    newSduDlBufferNode->SN = lcEntity->SN;

    // 更新LC实体的SN值
    lcEntity->SN = (lcEntity->SN + 1) % (int)pow(2, SN_LENGTH);

    // 释放bufferNode
    if (bufferNode->node.next == NULL) {
        bufferNode->node.pre->next = NULL;
    }
    else {
        bufferNode->node.pre->next = bufferNode->node.next;
    }
    free(bufferNode);
    lcEntity->dlBufferList.count--;
}

void GtLcBuildDataPdu() {
    // 遍历所有的T节点，检查为每个逻辑信道分配了多少资源，对应属性sched_len
    GtListNode* curTNode = &TNodeList.node;
    gt_u32 TNodeCount = TNodeList.count;
    
    // 异常情况判断：T节点链表为空
    if (TNodeCount == 0) {
        printf("\nTNodeList is empty\n");
        return;
    }
    
    for (gt_u32 i = 0; i < TNodeCount; i++)
    {
        curTNode = curTNode->next;

        for (gt_u8 LCID = 0; LCID < G_LCID_MAX; LCID++)
        {
            GtLogicChannel* logicChannel = &(((TNode*)curTNode)->dlLogicChannel[LCID]);

            if (logicChannel->sduDlBuffer.size == 0 || logicChannel->sduDlBuffer.sched_len == 0) break;

            // 遍历逻辑信道，组包（也就是添加头部）
            GtListNode* curSduDlBufferNode = &(logicChannel->sduDlBuffer.sduDlBufferList.node);
            //对于每一条逻辑信道，它都有一个sduDLBuffer，该数据结构中有一条链表，这个链表存储的是在该逻辑信道中的待向下发送的SDU队列
            for (gt_u32 i = 0; i < logicChannel->sduDlBuffer.sduDlBufferList.count; i++) {
                curSduDlBufferNode = curSduDlBufferNode->next;  //当前逻辑信道的sdu下行缓冲区的头结点
                gt_u32 remLen = ((GtLcSduDlBufferNode*)curSduDlBufferNode)->remLen;    // 剩余长度
                gt_u32 SO = ((GtLcSduDlBufferNode*)curSduDlBufferNode)->SO;            // 偏移量，指示该SDU已经被分段的长度
                gt_u8  lcMode = ((GtLcSduDlBufferNode*)curSduDlBufferNode)->lcMode;    // 指示传输模式: 0 m11, 1 m12, 2 m21, 3 m22, 4 m3
                gt_u32 SN = ((GtLcSduDlBufferNode*)curSduDlBufferNode)->SN;
                gt_u16 dataLen = 0;
                GtLcPduBufferNode* pduBufferNode = NULL;//pdu的节点链表，代表着某一个逻辑信道的pdu缓冲区
                if (lcMode == 3) {//这个sched_len是每一个逻辑信道会有一个，针对的是该逻辑信道的整个sdu队列
                    if (logicChannel->sduDlBuffer.sched_len <= mode22HeaderSize) {
                        printf("\nschedLen <= mode22HeaderSize\n");//该逻辑信道可以传输的数据量甚至小于一个22的头大小，那么就不用分配了
                        break;
                    }
                    else {
                        dataLen = (remLen > (logicChannel->sduDlBuffer.sched_len - mode22HeaderSize)) ? (logicChannel->sduDlBuffer.sched_len - mode22HeaderSize) : remLen;
                        pduBufferNode = malloc(mode22HeaderSize + dataLen);
                        pduBufferNode->node.next = NULL;
                        pduBufferNode->node.pre = NULL;
                        pduBufferNode->len = mode22HeaderSize + dataLen;
                        GtLcMode22Header pduHeader;
                        // 为LC Header赋值（对应的就是上面的mode22HeaderSize）
                        if (SO == 0 && dataLen == remLen)      pduHeader.SI = 0;
                        else if (SO == 0 && dataLen < remLen)  pduHeader.SI = 1;
                        else if (SO != 0 && dataLen < remLen)  pduHeader.SI = 2;
                        else if (SO != 0 && dataLen == remLen) pduHeader.SI = 3;
                        pduHeader.SO = SO;
                        pduHeader.SN = SN;
                        pduHeader.protocal = 0;

                        // 更新sduListNode对应的属性 curSduDlBufferNode代表的是某一逻辑信道对应的sdu缓冲区链表中的某一个节点
                        ((GtLcSduDlBufferNode*)curSduDlBufferNode)->SO += dataLen;//分配datalen的发送机会
                        ((GtLcSduDlBufferNode*)curSduDlBufferNode)->remLen -= dataLen;

                        // 组建PDU 每一个SDU都会分出一小块来组成PDU
                        memcpy(pduBufferNode->data, &pduHeader, mode22HeaderSize);
                        memcpy((gt_u8*)pduBufferNode->data + mode22HeaderSize, ((GtLcSduDlBufferNode*)curSduDlBufferNode)->data, dataLen);
                        //下述操作是将新创建的PDU放入该逻辑信道的PDU队列，所以说SDU队列中的每一个SDU都会组成一个PDU，只不过可能用的是全部sdu也可能只是部分sdu
                        GtListNode* curPduDlBufferNode = &logicChannel->pduDlBufferList.node;
                        gt_u32 pduDlBufferCount = logicChannel->pduDlBufferList.count;
                        for (gt_u32 i = 0; i < pduDlBufferCount; i++)
                        {
                            curPduDlBufferNode = curPduDlBufferNode->next;
                        }
                        curPduDlBufferNode->next = &pduBufferNode->node;
                        pduBufferNode->node.pre = curPduDlBufferNode;
                        pduBufferNode->node.next = NULL;
                        logicChannel->pduDlBufferList.count++;

                        // 修改条件判断属性 
                        logicChannel->sduDlBuffer.size -= dataLen;
                        logicChannel->sduDlBuffer.sched_len -= (dataLen + mode22HeaderSize);

                        // 当前节点的数据前移，根据remLen决定是否指向下一个SDU 这个remlen是每一个sdu链表节点会有一个
                        if (((GtLcSduDlBufferNode*)curSduDlBufferNode)->remLen != 0) {
                            memmove(((GtLcSduDlBufferNode*)curSduDlBufferNode)->data, (gt_u8*)((GtLcSduDlBufferNode*)curSduDlBufferNode)->data + dataLen, ((GtLcSduDlBufferNode*)curSduDlBufferNode)->remLen);
                            break;
                        }

                    }
                }
            }
            // 每次组包之后，即使sched_len有剩余，也将其置为0
            logicChannel->sduDlBuffer.sched_len = 0;
        }
    }
    //freeSduBufferListNode();
}

//void testfree(GtLogicChannel* logicChannel) {
//    GtListNode ttt = logicChannel->sduDlBuffer.sduDlBufferList.node;
//    free(ttt.next->next);
//}

//void testfree2() {
//    TNode* firstNode = (TNode*)TNodeList.node.next;
//    GtListNode* node = &(firstNode->dlLogicChannel[0].sduDlBuffer.sduDlBufferList.node);
//    free(node->next->next);
//}



