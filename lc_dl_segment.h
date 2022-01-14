#ifndef __LC_DL_SEGMENT_H__
#define __LC_DL_SEGMENT_H__

#include "test_gt_types.h"
#include "lc_entity_if.h"


//#include "gt_types.h"
#ifdef __cplusplus
extern "C" {
#endif
	/*
	 * 函数功能：基础服务层将数据交给LC层，通过TCID（基础服务层可见）和LC实体ID的对应关系，数据会被递交给某个特定的LC实体
	 * 问题：LC层如何维护TCID和LC实体ID之间的转换关系？
	 */
	void GtBsLcDataRecv(gt_void* msg, gt_u16 len);
	/*
	 * 函数功能：将LC实体下行缓冲区中的数据递交给对应的T节点的逻辑信道
	 */
	void GtLcLogicChannelDataPush(GtLcEntity* lcEntity, GtLcEntitybufferListNode* bufferNode);

	/*
	 * 函数功能：DMAC调用该函数，通知LC层资源分配结束，LC层开始组包
	 */
	void GtLcBuildDataPdu();

#ifdef __cplusplus
}
#endif

#endif // __LC_DL_SEGMENT_H__
