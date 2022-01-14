#ifndef __LC_HEADER_IF_H__
#define __LC_HEADER_IF_H__

#include "test_gt_types.h"
//#include "gt_types.h"
#pragma pack(4)

#ifdef __cplusplus
extern "C" {
#endif


	/*
	 * 字段含义说明：
	 * SI : 00(包含一个完整的SDU)；01(包含SDU第一个分段)；10(包含SDU的中间分段)；11(包含SDU的最后一个分段)
	 * SN : 指示LC SDU内包含的PDU按照传输顺序产生的编号，同一个SDU因分段生成的不同PDU在头部维护的SN号应该是一致的
	 */

	 // 传输模式2-1：添加LC头；LC层做状态报告；不允许对SDU进行切分
	typedef struct tagLcMode21Header {
		gt_u64        SI : 2;          // 分段指示
		gt_u64        rsv : 4;          // 预留
		gt_u64        SN : 18;         // 顺序号
		gt_u64        protocal : 16;         // 上层协议指示 
	} GtLcMode21Header;

	// 传输模式2-2：添加LC头；LC层做状态报告；不允许对SDU进行切分
	typedef struct tagLcMode22Header {
		gt_u64        SI : 2;          // 分段指示
		gt_u64        rsv : 4;          // 预留
		gt_u64        SN : 18;         // 顺序号
		gt_u64        protocal : 16;         // 上层协议指示 
		gt_u64        SO : 16;         // 分段偏移值
	} GtLcMode22Header;



#ifdef __cplusplus
}
#endif

#endif // __LC_HEADER_IF_H__





