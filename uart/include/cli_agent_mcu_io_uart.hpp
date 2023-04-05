#ifndef __CLI_AGENT_MCU_IO_UART__
#define __CLI_AGENT_MCU_IO_UART__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>  
#include <errno.h>  
#include <arpa/inet.h>  
#include <signal.h>
#include <stdarg.h>
#include <string>
#include <memory>

#include "cli_agent_mcu_io_device.hpp"

namespace maix {

    class CCliAgentMcuIoUart : public CCliAgentMcuIoDevice
    {
    public:
        CCliAgentMcuIoUart();
        virtual ~CCliAgentMcuIoUart();

        virtual bool Configure(T_uMcuIoConfig &Config);

        virtual int SendBuf(unsigned char *pBuf, int iLen);
        virtual int RecvBuf(unsigned char *pBuf, int iLen, int iTimeout);

    private:
        bool Open();
        bool SetOpt();

    private:
        int m_fd;
        T_sMcuIoUartConfig m_IoConfig;
    };
}
#endif /* __CLI_AGENT_MCU_IO_UART__ */