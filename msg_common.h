#ifndef __MSG_COMMON_H__
#define __MSG_COMMON_H__

#include "test_gt_types.h"

#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(4)

	typedef enum
	{
		TGT_XRC,
		TGT_LC_DL,
		TGT_LC_UL,
		TGT_MAC_SCH,
		TGT_MAC_DL,
		TGT_MAC_UL,
		TGT_PHY_AGT,
		TGT_MAX
	}GtMsgTarget;


	typedef struct
	{
		GtMsgTarget src;
		GtMsgTarget dst;
		gt_u16      type;
		gt_u16      len;
		gt_u16      transid;
	} GtMsgHeader;

	// 填充头部
	// void GtFillMsgHeader(GtMsgHeader* header, GtMsgTarget src, GtMsgTarget dst, gt_u16 type, gt_u16 len);

	// void GtSendMsg(GtMsgTarget dst, void* data, gt_u32 len);

#ifdef __cplusplus
}
#endif

#endif // __TEST_H__
