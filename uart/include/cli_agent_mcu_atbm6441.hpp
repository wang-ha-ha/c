#ifndef __CLI_AGENT_MCU_ATBM6441__
#define __CLI_AGENT_MCU_ATBM6441__

namespace maix {
    class CCliAgentMcuAtbm6441: public CCliAgentMcu
    {
    public:
        CCliAgentMcuAtbm6441();
        ~CCliAgentMcuAtbm6441();

        virtual bool Configure(const char *UartPath, int BaudRate);

    private:

    };
}
#endif /* __CLI_AGENT_MCU_ATBM6441__ */