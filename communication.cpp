#include "define.h"

#include "device.h"
#include "usbcommand.h"
#include "command.h"
#include "crypt/crypt_user.h"

#include "communication.h"

/**
 * @brief 构造函数，初始化通信对象
 * @param devname 设备名称
 */
CCommunication::CCommunication(const std::string& devname)
{
    // 设置连接模式为USB连接模式
    m_nConnectionMode = USB_CON_MODE;
    // 初始化USB句柄为空
    m_hUsbHandle = nullptr;
    // 保存设备名称
    m_devname = devname;
}

/**
 * @brief 析构函数，释放USB资源
 */
CCommunication::~CCommunication()
{
    // 反初始化USB SCSI设备
    USBSCSIDeInit();
}

/**
 * @brief 初始化连接
 * @return 连接结果，成功返回CONNECTION_SUCCESS，失败返回ERR_USB_OPEN_FAIL
 */
int CCommunication::Run_InitConnection()
{
    // 初始化源设备ID和目标设备ID为0
    m_bySrcDeviceID = 0;
    m_byDstDeviceID = 0;

    // 初始化USB SCSI设备，如果失败则关闭连接并返回错误码
    if (USBSCSIInit(m_devname.c_str()) == FALSE)
    {
        CloseConnection();
        return ERR_USB_OPEN_FAIL;
    }

    return CONNECTION_SUCCESS;
}

/**
 * @brief 获取指纹图像
 * @return 操作结果，成功返回RESPONSE_RET，失败返回ERR_CONNECTION
 */
int CCommunication::Run_GetImage(void)
{
    // 初始化命令包，请求获取指纹图像
    InitCmdPacket(CMD_GET_IMAGE, m_bySrcDeviceID, m_byDstDeviceID, NULL, 0);
    
    // 发送命令包，如果失败则返回连接错误码
    if (!USB_SendPacket(m_hUsbHandle, CMD_GET_IMAGE, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    return RESPONSE_RET;
}

/**
 * @brief 检测指纹
 * @param p_pnDetectResult 指向存储检测结果的指针
 * @return 操作结果，成功返回ERR_SUCCESS，失败返回相应错误码
 */
int CCommunication::Run_FingerDetect(int* p_pnDetectResult)
{
    // 初始化命令包，请求检测指纹
    InitCmdPacket(CMD_FINGER_DETECT, m_bySrcDeviceID, m_byDstDeviceID, NULL, 0);
    
    // 发送命令包，如果失败则返回连接错误码
    if (!USB_SendPacket(m_hUsbHandle, CMD_FINGER_DETECT, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    // 如果响应结果不是成功，则返回响应结果
    if (RESPONSE_RET != ERR_SUCCESS)
        return RESPONSE_RET;
    
    // 将检测结果存储到指针指向的位置
    *p_pnDetectResult = g_pRcmPacket->m_abyData[0];
    return ERR_SUCCESS;
}

/**
 * @brief 存储指纹特征
 * @param p_nTmplNo 模板编号
 * @param p_nRamBufferID RAM缓冲区ID
 * @param p_pnDupTmplNo 指向存储重复模板编号的指针
 * @return 操作结果，成功返回ERR_SUCCESS，失败返回相应错误码
 */
int CCommunication::Run_StoreChar(int p_nTmplNo, int p_nRamBufferID, int* p_pnDupTmplNo)
{
    BYTE w_abyData[4];
    
    // 拆分模板编号为低字节和高字节
    w_abyData[0] = LOBYTE(p_nTmplNo);
    w_abyData[1] = HIBYTE(p_nTmplNo);
    w_abyData[2] = p_nRamBufferID;
    
    // 初始化命令包，请求存储指纹特征
    InitCmdPacket(CMD_STORE_CHAR, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 3);
    
    // 发送命令包，如果失败则返回连接错误码
    if (!USB_SendPacket(m_hUsbHandle, CMD_STORE_CHAR, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    // 如果响应结果不是成功，则返回响应结果
    if (RESPONSE_RET != ERR_SUCCESS)
        return RESPONSE_RET;
    
    // 将重复模板编号存储到指针指向的位置
    *p_pnDupTmplNo = MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);
    return ERR_SUCCESS;
}

/**
 * @brief 删除指纹特征
 * @param p_nSTmplNo 起始模板编号
 * @param p_nETmplNo 结束模板编号
 * @return 操作结果，成功返回RESPONSE_RET，失败返回ERR_CONNECTION
 */
int CCommunication::Run_DelChar(int p_nSTmplNo, int p_nETmplNo)
{
    BYTE w_abyData[4];
    
    // 拆分起始模板编号和结束模板编号为低字节和高字节
    w_abyData[0] = LOBYTE(p_nSTmplNo);
    w_abyData[1] = HIBYTE(p_nSTmplNo);
    w_abyData[2] = LOBYTE(p_nETmplNo);
    w_abyData[3] = HIBYTE(p_nETmplNo);
    
    // 初始化命令包，请求删除指纹特征
    InitCmdPacket(CMD_DEL_CHAR, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 4);
    
    // 发送命令包，如果失败则返回连接错误码
    if (!USB_SendPacket(m_hUsbHandle, CMD_DEL_CHAR, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    return RESPONSE_RET;
}

/**
 * @brief 获取空模板ID
 * @param p_nSTmplNo 起始模板编号
 * @param p_nETmplNo 结束模板编号
 * @param p_pnEmptyID 指向存储空模板ID的指针
 * @return 操作结果，成功返回ERR_SUCCESS，失败返回相应错误码
 */
int CCommunication::Run_GetEmptyID(int p_nSTmplNo, int p_nETmplNo, int* p_pnEmptyID)
{
    BYTE w_abyData[4];
    
    // 拆分起始模板编号和结束模板编号为低字节和高字节
    w_abyData[0] = LOBYTE(p_nSTmplNo);
    w_abyData[1] = HIBYTE(p_nSTmplNo);
    w_abyData[2] = LOBYTE(p_nETmplNo);
    w_abyData[3] = HIBYTE(p_nETmplNo);
    
    // 初始化命令包，请求获取空模板ID
    InitCmdPacket(CMD_GET_EMPTY_ID, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 4);
    
    // 发送命令包，如果失败则返回连接错误码
    if (!USB_SendPacket(m_hUsbHandle, CMD_GET_EMPTY_ID, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    // 如果响应结果不是成功，则返回响应结果
    if (RESPONSE_RET != ERR_SUCCESS)
        return RESPONSE_RET;
    
    // 将空模板ID存储到指针指向的位置
    *p_pnEmptyID = MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);
    return ERR_SUCCESS;
}

/**
 * @brief 生成指纹特征
 * @param p_nRamBufferID RAM缓冲区ID
 * @return 操作结果，成功返回RESPONSE_RET，失败返回ERR_CONNECTION
 */
int CCommunication::Run_Generate(int p_nRamBufferID)
{
    BYTE w_abyData[1];
    
    // 存储RAM缓冲区ID
    w_abyData[0] = p_nRamBufferID;
    
    // 初始化命令包，请求生成指纹特征
    InitCmdPacket(CMD_GENERATE, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 1);
    
    // 发送命令包，如果失败则返回连接错误码
    if (!USB_SendPacket(m_hUsbHandle, CMD_GENERATE, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    return RESPONSE_RET;
}

/**
 * @brief 合并指纹特征
 * @param p_nRamBufferID RAM缓冲区ID
 * @param p_nMergeCount 合并次数
 * @return 操作结果，成功返回RESPONSE_RET，失败返回ERR_CONNECTION
 */
int CCommunication::Run_Merge(int p_nRamBufferID, int p_nMergeCount)
{
    BYTE w_abyData[2];
    
    // 存储RAM缓冲区ID和合并次数
    w_abyData[0] = p_nRamBufferID;
    w_abyData[1] = p_nMergeCount;
    
    // 初始化命令包，请求合并指纹特征
    InitCmdPacket(CMD_MERGE, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 2);
    
    // 发送命令包，如果失败则返回连接错误码
    if (!USB_SendPacket(m_hUsbHandle, CMD_MERGE, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    return RESPONSE_RET;
}

/**
 * @brief 搜索指纹特征
 * @param p_nRamBufferID RAM缓冲区ID
 * @param p_nStartID 起始搜索ID
 * @param p_nSearchCount 搜索数量
 * @param p_pnTmplNo 指向存储匹配模板编号的指针
 * @param p_pnLearnResult 指向存储匹配结果的指针
 * @return 操作结果，成功返回ERR_SUCCESS，失败返回相应错误码
 */
int CCommunication::Run_Search(int p_nRamBufferID, int p_nStartID, int p_nSearchCount, int* p_pnTmplNo, int* p_pnLearnResult)
{
    BYTE w_abyData[5];
    
    // 存储RAM缓冲区ID、起始搜索ID和搜索数量
    w_abyData[0] = p_nRamBufferID;
    w_abyData[1] = LOBYTE(p_nStartID);
    w_abyData[2] = HIBYTE(p_nStartID);
    w_abyData[3] = LOBYTE(p_nSearchCount);
    w_abyData[4] = HIBYTE(p_nSearchCount);
    
    // 初始化命令包，请求搜索指纹特征
    InitCmdPacket(CMD_SEARCH, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 5);
    
    // 发送命令包，如果失败则返回连接错误码
    if (!USB_SendPacket(m_hUsbHandle, CMD_SEARCH, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    // 如果响应结果不是成功，则返回响应结果
    if (RESPONSE_RET != ERR_SUCCESS)
        return RESPONSE_RET;
    
    // 将匹配模板编号和匹配结果存储到指针指向的位置
    *p_pnTmplNo = MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);
    *p_pnLearnResult = g_pRcmPacket->m_abyData[2];
    
    return ERR_SUCCESS;
}

/**
 * @brief 关闭连接
 */
void CCommunication::CloseConnection()
{
    // 反初始化USB SCSI设备
    USBSCSIDeInit();
}