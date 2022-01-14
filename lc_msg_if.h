#ifndef __LC_MSG_IF_H__
#define __LC_MSG_IF_H__

#include "test_gt_types.h"
#include "msg_common.h"
//#include "gt_types.h"

#ifdef _cplusplus
extern "C" {
#endif
#pragma pack(4)
    // BS和LC之间的消息类型定义枚举，这部分内容需要根据接口文档修改
#define BS_LC_MSG_TYPE_BASE 0x5000 

    typedef enum {
        BS_LC_DATA_REQ = BS_LC_MSG_TYPE_BASE + 0x00,    // 基础服务层请求LC层传输数据
        BS_LC_DATA_IND = BS_LC_MSG_TYPE_BASE + 0x01,    // 数据接收指示
        BS_LC_MSG_TYPE_BUTT
    }BSLC_MSG_TYPE_ENUM;

    /*
     * 基础服务层请求LC层传输数据，BS->LC，对应原语BS_LC_DATA_REQ
     */
    typedef struct {
        GtMsgHeader   header;              // 消息头
        gt_u8         rsv[3];              // 保留位，根据TCID占多少字节数变化
        // gt_u16        TCID;             // TCID和LC entityID有映射关系 ----> 打桩，暂时用lcEntity来代替
        gt_u8         lcEntityId;
        gt_u32        dataLen;             // 柔性数组中数据的长度          
        gt_u8         data[0];             // 柔性数据保存数据
    }GtBsLcDataDlReqMsg;

    // L3和LC之间的消息类型定义枚举
#define L3_LC_MSG_TYPE_BASE 0x4000

    typedef enum {
        L3_LC_SETUP_REQ = L3_LC_MSG_TYPE_BASE + 0x00,      // LC用户实体建立请求
        L3_LC_SETUP_RSP = L3_LC_MSG_TYPE_BASE + 0x01,      // LC用户实体建立响应
        L3_LC_RECFG_REQ = L3_LC_MSG_TYPE_BASE + 0x02,      // LC用户实体重配置请求
        L3_LC_RECFG_RSP = L3_LC_MSG_TYPE_BASE + 0x03,      // LC用户实体重配置响应
        L3_LC_RELEASE_REQ = L3_LC_MSG_TYPE_BASE + 0x04,    // LC用户实体释放请求
        L3_LC_RELEASE_RSP = L3_LC_MSG_TYPE_BASE + 0x05,    // LC用户实体释放响应
        L3_LC_DATA_REQ = L3_LC_MSG_TYPE_BASE + 0x06,       // SRB消息发送请求
        L3_LC_DATA_IND = L3_LC_MSG_TYPE_BASE + 0x07,       // SRB消息接收指示
        L3_LC_MSG_TYPE_BUTT
    }L3LC_MSG_TYPE_ENUM;


    /*
     * 用户建立请求，xRC->LC，对应原语L3_LC_SETUP_REQ
     * 用户修改请求，xRC->LC，对应原语L3_LC_RECFG_REQ
     */
    typedef struct {
        GtMsgHeader   header;              // 消息头
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
    }GtLcSetupReqMsg, GtLcReconfReqMsg;

    /* L3向LC发送用户释放请求，L3_LC_RELEASE_REQ */
    typedef struct
    {
        GtMsgHeader   header;
        gt_u8         domainID;            // 域ID，取值0~3
        gt_u8         lcEntityID;          // LC实体ID
        gt_u8         rsv[2];
    }GtLcReleaseReqMsg;


    /*
     * 用户建立响应，LC->xRC，对应原语L3_LC_SETUP_RSP
     * 用户修改请求响应，LC->xRC，对应原语L3_LC_RECFG_RSP
     */
    typedef struct
    {
        GtMsgHeader   header;
        gt_u8         domainID;            // 域ID，取值0~3
        gt_u8         result;              // 0表示成功; 其它表示错误码
        gt_u8         rsv[2];
    }GtLcSetupRspMsg, GtLcReconfRspMsg;

    /*
     * 用户释放响应，LC->xRC，对应原语L3_LC_RELEASE_RSP
     */
    typedef struct
    {
        GtMsgHeader   header;
        gt_u8         domainID;            // 域ID，取值0~3
        gt_u8         lcEntityID;          // LC实体ID
        gt_u8         result;              // 0表示成功; 其它表示错误码
        gt_u8         rsv;
    }GtLcReleaseRspMsg;


    /* L3向LC发送xRC信令，xRC->LC，对应原语L3_LC_DATA_REQ */
    typedef struct
    {
        GtMsgHeader   header;
        gt_u8         domainID;            // 域ID，取值0~3
        gt_u8         uuMsgType;
        gt_u8         lcEntityID;          // LC实体ID
        gt_u8         rsv1;                // 保留位
        gt_u16        dataLen;             // SRB信令消息长度
        gt_u8         rsv2[2];             // 保留位
        gt_u8         xrcData[0];          // 柔性数组，指示的是信令payload
    }GtLcSignalReqMsg;

    /* LC接收到的xRC信令递交给xRC层，对应原语L3_LC_DATA_IND */
    typedef struct
    {
        GtMsgHeader   header;
        gt_u8         domainID;            // 域ID，取值0~3
        gt_u8         lcEntityID;          // LC实体ID
        gt_u16        dataLen;             // SRB信令消息长度
        gt_u8         xrcData[0];          // 柔性数组，指示的是信令payload
    }GtLcSignalIndMsg;

#ifdef _cplusplus
}
#endif

#endif
