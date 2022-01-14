#ifndef __LOGICCHANNEL_IF_H__
#define __LOGICCHANNEL_IF_H__

#include "test_list_if.h"
#include "test_gt_types.h"

#ifdef __cplusplus
extern "C" {
#endif


#pragma pack(4)

	typedef struct {
		GtListNode    node;
		gt_u32        remLen;      // 当前的剩余长度，初始化为SDU的总长度
		gt_u8         lcMode;      // 指示传输模式: 0 m11, 1 m12, 2 m21, 3 m22, 4 m3
		gt_u8         rsv[3];      // 保留位
		gt_u32        SN;          // 同一个SDU对应的所有PDU的头部SN值相同
		gt_u32        SO;          // 偏移量，该值代表对应SDU已经切分出去的比特数，SO + len == SDU的总长度
		gt_u8         data[0];     // 柔性数组
	} GtLcSduDlBufferNode;

	typedef struct
	{
		GtListNode    node;
		gt_u32        len;         // 柔性数组中数据的长度
		gt_u8         data[0];     // 柔性数组 
	}GtLcPduBufferNode;

	typedef struct {
		gt_u16                size;                    // 逻辑信道中当前缓存的数据量，对应sduDlBufferList的所有数据
		gt_u16                sched_len;               // 该逻辑信道可用于组包的数据长度，由DMAC模块计算后写入
		GtList                sduDlBufferList;         // 该链表对应的结点类型是GtLcSduDlBufferNode，存放LC SDU
	}GtLcSduDlBuffer;

	typedef struct {
		gt_u16                priority;                // 逻辑信道优先级
		gt_u16                prioritisedBitSize;      // 逻辑信道优先比特量
		GtLcSduDlBuffer       sduDlBuffer;             // LC DL SDU buffer
		GtList                pduDlBufferList;         // 该链表对应的结点类型是GtLcPduBufferNode，存放下行LC PDU
		GtList                pduUlBufferList;         // 该链表对应的结点类型是GtLcPduBufferNode，存放上行LC PDU
	}GtLogicChannel;

#ifdef __cplusplus
}
#endif

#endif // __LOGICCHANNEL_IF_H__


