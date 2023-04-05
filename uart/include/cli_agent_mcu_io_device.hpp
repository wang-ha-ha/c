#ifndef __CLI_AGENT_MCU_IO_DEVICE__
#define __CLI_AGENT_MCU_IO_DEVICE__

#include <string>
#include <memory>

namespace maix {

    typedef struct _McuIoUartConfig
    {
        int iBaudRate;
        int iBits;
        int iStop;
        int iInterval;
        char cParity;
        char strUartPath[256];
    }T_sMcuIoUartConfig;

    typedef struct _McuIoTcpConfig
    {
        char strIP[20];
        int iPort;
    }T_sMcuIoTcpConfig;

    typedef union _uMcuIoConfig
    {    
        T_sMcuIoUartConfig sUartConfig;
        T_sMcuIoTcpConfig sTcpConfig;
    }T_uMcuIoConfig;

    class CCliAgentMcuIoDevice
    {
    public:
        CCliAgentMcuIoDevice();
        virtual ~CCliAgentMcuIoDevice();

        virtual bool Configure(T_uMcuIoConfig &Config) = 0;

        virtual int SendBuf(unsigned char *pBuf, int iLen) = 0;
        virtual int RecvBuf(unsigned char *pBuf, int iLen, int iTimeout) = 0;
    };
}
#endif /* __CLI_AGENT_MCU_IO_DEVICE__ */