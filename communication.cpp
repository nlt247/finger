#include "define.h"

#include "device.h"
#include "usbcommand.h"
#include "command.h"
#include "crypt/crypt_user.h"

#include "communication.h"

CCommunication::CCommunication(const std::string& devname)
{
    m_nConnectionMode = USB_CON_MODE;
    m_hUsbHandle = nullptr;
    m_devname = devname;
}

CCommunication::~CCommunication()
{
    USBSCSIDeInit();
}

int CCommunication::Run_InitConnection()
{
    m_bySrcDeviceID = 0;
    m_byDstDeviceID = 0;

    if (USBSCSIInit(m_devname.c_str()) == FALSE)
    {
        CloseConnection();
        return ERR_USB_OPEN_FAIL;
    }

    return CONNECTION_SUCCESS;
}

// [其余方法保持不变，只需将QObject转换为std::string]