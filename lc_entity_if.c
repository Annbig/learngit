#include "lc_msg_if.h"
#include "lc_entity_if.h"
#include "logicChannel_if.h"
#include "stdlib.h" 
#include "math.h"


extern GtList lcEntityList;
#define SN_LENGTH  18

void freeList(GtListNode* curNode, gt_u32 count) {
    for (gt_u32 i = 0; i < count; i++) {
        curNode = curNode->next;
    }
    for (gt_u32 i = 0; i < count; i++) {
        curNode = curNode->pre;
        free(curNode->next);
        curNode->next = NULL;
    }
}

/*
 * 函数功能：根据LC实体ID返回该LC实体在链表中对应的位置
 * 返回值：
 *  （1）在链表中找到了对应的节点：curLen和curPos分别表示节点的位置和节点的地址
 *  （2）在链表中没找到对应的节点：curLen == count  + 1, curPos指向链表最后一个节点
 */

GtListNode* getPositionById(gt_u8 entityId, gt_u32* curLen)
{
    gt_u32 count = lcEntityList.count;
    GtListNode* curPos = &lcEntityList.node;
    if (curPos->next == NULL) {
        (*curLen)++;
        return curPos;
    }
    for (gt_u32 i = 0; i < count; i++)
    {
        curPos = curPos->next;
        (*curLen)++;
        if (((GtLcEntity*)curPos)->lcEntityID == entityId) break;
    }

    // 修复BUG，这里并未考虑头结点的情况，针对curPos->next == NULL或者count == 0需要做特殊处理
    if (*curLen == count && ((GtLcEntity*)curPos)->lcEntityID != entityId) // 该条件表明的就是没找到的情况
    {
        (*curLen)++;
    }
    return curPos;
}

/*
 * 函数功能说明：删除LC实体链表中的某个节点并释放内存
 */
void releaseByPos(GtListNode* curPos, gt_u32 curLen)
{
    if (curLen == lcEntityList.count) // 释放的是最后一个节点
    {
        curPos->pre->next = NULL;
    }
    else {                           // 释放的是中间节点 
        curPos->pre->next = curPos->next;
        curPos->next->pre = curPos->pre;
    }
    lcEntityList.count--;

    // 释放dlBufferList
    GtListNode* curNode = &(((GtLcEntity*)curPos)->dlBufferList.node);
    freeList(curNode, ((GtLcEntity*)curPos)->dlBufferList.count);
    ((GtLcEntity*)curPos)->dlBufferList.count = 0;
    // 释放ulBufferList
    curNode = &(((GtLcEntity*)curPos)->ulBufferList.node);
    freeList(curNode, ((GtLcEntity*)curPos)->ulBufferList.count);
    ((GtLcEntity*)curPos)->ulBufferList.count = 0;
    // 释放umRxWindow
    for (gt_u32 i = 0; i < (gt_u32)pow(2, SN_LENGTH); i++) {
        freeList(&(((GtLcEntity*)curPos)->umRxWindow.ulPduArray[i].node),
            ((GtLcEntity*)curPos)->umRxWindow.ulPduArray[i].count);
        ((GtLcEntity*)curPos)->umRxWindow.ulPduArray[i].count = 0;
    }
    free(((GtLcEntity*)curPos)->umRxWindow.ulPduArray);
}

/*
 * 函数功能：根据msg的内容为LC实体赋值
 */
void lcEntityAssignment(GtLcEntity* lcEntity, gt_void* msg)
{
    GtLcSetupReqMsg* buildMsg = (GtLcSetupReqMsg*)msg;
    lcEntity->domainID = buildMsg->domainID;
    lcEntity->rbId = buildMsg->rbId;
    lcEntity->rbType = buildMsg->rbType;
    lcEntity->lcId = buildMsg->lcId;
    lcEntity->lcEntityID = buildMsg->lcEntityID;
    lcEntity->lcDirection = buildMsg->lcDirection;
    lcEntity->lcMode = buildMsg->lcMode;
    lcEntity->encryption = buildMsg->encryption;
    lcEntity->integrityProtection = buildMsg->integrityProtection;
    lcEntity->trr = buildMsg->trr;
    lcEntity->tPollPeriod = buildMsg->tPollPeriod;
    lcEntity->tPollRetransmit = buildMsg->tPollRetransmit;
    lcEntity->TNodeID = buildMsg->TNodeID;
    lcEntity->SN = 0;
    lcEntity->dlBufferList.node.next = NULL;
    lcEntity->dlBufferList.node.pre = NULL;
    lcEntity->dlBufferList.count = 0;
    lcEntity->ulBufferList.node.next = NULL;
    lcEntity->ulBufferList.node.pre = NULL;
    lcEntity->ulBufferList.count = 0;
    lcEntity->umRxWindow.RX_Next_Highest = 0;
    lcEntity->umRxWindow.RX_Next_Reassembly = 0;
    lcEntity->umRxWindow.RX_Timer_Trigger = 0;
    lcEntity->umRxWindow.isTrigger = 0;
    lcEntity->umRxWindow.t_Reassembly = 0;
    lcEntity->umRxWindow.ulPduArray = malloc(sizeof(umRecvWindowElement) * (int)pow(2, SN_LENGTH));
    for (gt_u32 i = 0; i < (gt_u32)pow(2, SN_LENGTH); i++) {
        lcEntity->umRxWindow.ulPduArray[i].node.pre = NULL;
        lcEntity->umRxWindow.ulPduArray[i].node.next = NULL;
        lcEntity->umRxWindow.ulPduArray[i].finish = 0;
        lcEntity->umRxWindow.ulPduArray[i].totalLen = 0;
        lcEntity->umRxWindow.ulPduArray[i].curLen = 0;
        lcEntity->umRxWindow.ulPduArray[i].count = 0;
    }
}

// 新建LC实体
void GtLcEntitySetup(gt_void* msg, gt_u16 len) {

    GtLcSetupReqMsg* buildMsg = (GtLcSetupReqMsg*)msg;

    gt_u8 entityId = buildMsg->lcEntityID;
    gt_u32 curLen = 0;
    GtListNode* curPos = getPositionById(entityId, &curLen);
    GtLcSetupRspMsg rspMsg;

    if (curLen > lcEntityList.count) // 代表该lcEntityID可用
    {
        rspMsg.result = 0;
        GtLcEntity* newEntity = (GtLcEntity*)malloc(sizeof(GtLcEntity));

        curPos->next = &(newEntity->node);
        newEntity->node.pre = curPos;
        newEntity->node.next = NULL;
        lcEntityList.count++;

        // 为新的LC实体赋值
        lcEntityAssignment(newEntity, msg);
        rspMsg.domainID = newEntity->domainID;
    }
    else {
        rspMsg.result = 1;
        // 用于测试
        printf("\nLcEntityID = %d already exists", entityId);
    }

    //// 组响应消息并添加到消息队列
    //GtFillMsgHeader(&rspMsg.header,
    //    TGT_LC_UL,         // src ?
    //    TGT_XRC,           // dst
    //    L3_LC_SETUP_RSP,   // type
    //    sizeof(GtLcSetupRspMsg));
    //GtSendMsg(TGT_XRC, &rspMsg, sizeof(GtLcSetupRspMsg));
}



// 释放LC实体
void GtLcEntityRelease(gt_void* msg, gt_u16 len) {

    GtLcReleaseReqMsg* releaseMsg = (GtLcReleaseReqMsg*)msg;

    gt_u8 entityId = releaseMsg->lcEntityID;
    gt_u32        curLen = 0;
    GtListNode* curPos = getPositionById(entityId, &curLen);
    GtLcReleaseRspMsg rspMsg;

    // 在链表中没有找到对应的实体
    // 对应两种情况：（1）lcEntityList是一个空链表；（2）lcEntityList不是一个空链表但找不到对应的实体。
    if (curLen > lcEntityList.count) {
        rspMsg.result = 1;
        // 用于测试
        printf("\n不存在lcEntityID == %d的LC实体\n", entityId);
    }
    else { // 在链表中找到了对应的实体
        rspMsg.result = 0;
        rspMsg.domainID = ((GtLcEntity*)curPos)->domainID;
        rspMsg.lcEntityID = entityId;
        releaseByPos(curPos, curLen);
        free((GtLcEntity*)curPos);
    }



    //// 组响应消息
    //GtFillMsgHeader(&rspMsg.header,
    //    TGT_LC_UL,         // src ?
    //    TGT_XRC,           // dst
    //    L3_LC_RELEASE_RSP, // type
    //    sizeof(GtLcReleaseRspMsg));
    //GtSendMsg(TGT_XRC, &rspMsg, sizeof(GtLcReleaseRspMsg));

}






