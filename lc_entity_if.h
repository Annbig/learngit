#ifndef __LC_ENTITY_IF_H__
#define __LC_ENTITY_IF_H__

#include "test_list_if.h"
#include "test_gt_types.h"
//#include "gt_types.h"

#ifdef __cplusplus
extern "C" {
#endif


#pragma pack(4)

	GtList lcEntityList;   // LC实体链表

	typedef struct {
		GtListNode  node;             // 维护在链表中的位置
		gt_u32      len;              // data数组的长度
		gt_u8       data[0];
	} GtLcEntitybufferListNode;


	// 数组元素链表节点的定义
	typedef struct
	{
		GtListNode    node;
		gt_u32        SO;                // 与LC header中的SO保持一致
		gt_u32        len;
		gt_u8         data[0];
	}umRecvWinEleListNode;

	// 数组元素的定义
	typedef struct
	{
		GtListNode    node;
		gt_u32        finish;            // finish == 1表示已经重组完成向上递交，finish == 0表示还未重组完成，初始化为0
		gt_u32        totalLen;          // 总长度，初始化为0，拿到SI == 11的分段之后更新这个字段，更新的公式为totalLen = SO + len
		gt_u32        curLen;            // 当前长度，每次添加一个SDU分段就更新，初始化为0
		gt_u32        count;             // 链表中节点的数量
	}umRecvWindowElement;

	// UM接收实体重排序窗口的定义
	typedef struct
	{
		gt_u32               RX_Next_Highest;     // 重组窗的上沿，初始化为0        
		gt_u32               RX_Next_Reassembly;  // 重排序窗口内还未接收的拥有最小SN的UMD PDU，初始化为0
		gt_u32               RX_Timer_Trigger;    // 用于定时器触发后在窗口中标记的索引，初始化为0
		gt_u8                isTrigger;           // 1表示定时器已经超时，初始化为0，只有当isTrigger == 1时，RX_Timer_Trigger和t_Reassembly才有意义
		gt_u8                rsv[3];
		gt_u32               t_Reassembly;        // 定时器基于freeRTOS, 考虑删除该属性
		umRecvWindowElement* ulPduArray;          // 指向的长度为sizeof(umRxWindowElement) * pow(2, 18);这里长度是固定的，所以也就不必再单独指定len
	}umRecvWindow;

	// LC实体的定义
	typedef struct tagLcEntity {
		GtListNode    node;                // 维护在链表中的位置
		gt_u8         domainID;            // 域ID，取值0~3
		gt_u8         rbId;                // RB ID
		gt_u8         rbType;              // RB类型，0 SRB，1 DRB
		gt_u8         lcId;                // 逻辑信道ID
		gt_u8         lcEntityID;          // LC实体ID
		gt_u8         lcDirection;         // 0 uplinkOnly; 1 downlinkOnly; 2 both
		gt_u8         lcMode;              // 指示传输模式: 0 m11, 1 m12, 2 m21, 3 m22, 4 m3
		gt_u8         rsv;                 // 保留
		gt_u8         encryption;          // 指示是否加密，0 false，1 true
		gt_u8         integrityProtection; // 指示是否进行完整性保护，0 false，1 true
		gt_u16        trr;                 // 取值范围为(0...4095)，对应t-RR，用于包重组和重排序计时器，具体取值取决于Gnode配置
		gt_u16        tPollPeriod;         // 取值范围为(0...4095)，查询周期
		gt_u16        tPollRetransmit;     // 取值范围为(0...4095)，查询重传时间
		gt_u32        TNodeID;             // TNodeID
		gt_u32        SN;                  // 用于分段与重组
		GtList        dlBufferList;        // 该链表存储的是来自基础服务层的数据，链表中的节点类型是GtLcEntitybufferListNode
		GtList        ulBufferList;        // 接收完整SDU的上行数据缓冲区，链表中的节点类型是GtLcEntitybufferListNode
		umRecvWindow  umRxWindow;          // mode 22的接收窗口
		//amSendWindow  amTxWindow;          // mode 12的发送窗口
		//amRecvWindow  amRxWindow;          // mode 12的接收窗口
	} GtLcEntity;

	/*
	 * 函数功能：新建LC实体
	 */
	void GtLcEntitySetup(gt_void* msg, gt_u16 len);

	/*
	 * 函数功能：重配置LC实体
	 */
	void GtLcEntityReconfig(gt_void* msg, gt_u16 len);

	/*
	 * 函数功能：释放LC实体
	 */
	void GtLcEntityRelease(gt_void* msg, gt_u16 len);

#ifdef __cplusplus
}
#endif

#endif // __LC_ENTITY_IF_H__

