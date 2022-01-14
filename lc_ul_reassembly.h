#ifndef __LC_UL_REASSEMBLY_H__
#define __LC_UL_REASSEMBLY_H__



//#include "gt_types.h"
#include "lc_entity_if.h"  
#include "logicChannel_if.h"
#include "test_TNode.h"


/* Kernel includes. */
//#include "FreeRTOS.h"
//#include "task.h"
//#include "timers.h"


#pragma pack(4)

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * 函数功能：从逻辑信道中取数并解析
     */
    void getLcPdu();

    /*
     * 函数功能：UM模式重组接收窗口
     */
    void umAssembly(GtLcEntity* lcEntity, GtLcPduBufferNode* lcUlPdu);

#ifdef __cplusplus
}
#endif

#endif //__LC_UL_REASSEMBLY_H__
