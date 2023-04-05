#ifndef __CLI_AGENT_MCU__
#define __CLI_AGENT_MCU__

#include <string>
#include <memory>

#include "cli_agent_mcu_io_device.hpp"

namespace maix {
    class CCliAgentMcu
    {
    public:
        CCliAgentMcu();
        ~CCliAgentMcu();

        void SetIO(CCliAgentMcuIoDevice *IoDevice);
        CCliAgentMcuIoDevice *GetIO();

    public:
        

    private:
        CCliAgentMcuIoDevice *m_IoDevice;
    };

    class CCliAgentMcuFactory
    {
    public:
        static CCliAgentMcu *GetInstance(std::string type);

    private:
        CCliAgentMcuFactory();
        ~CCliAgentMcuFactory();

    private:
        static CCliAgentMcu *m_instance;
    };
}
#endif /* __CLI_AGENT_MCU__ */