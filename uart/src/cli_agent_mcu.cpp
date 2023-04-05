#include "cli_agent_mcu.hpp"
#include "cli_agent_mcu_bl616.hpp"
#include "cli_agent_mcu_atbm6441.hpp"

namespace maix {
    CCliAgentMcu *CCliAgentMcuFactory::m_instance = NULL;

    CCliAgentMcu *CCliAgentMcuFactory::GetInstance(std::string type)
    {
        if(m_instance != NULL)
        {
            return m_instance;
        }

        if(type.compare("bl616") == 0)
        {
            m_instance = new CCliAgentMcuBl616();
        }
        else if(type.compare("atbm6441") == 0)
        {
            m_instance = new CCliAgentMcuAtbm6441(); 
        }

        return m_instance;
    }

    CCliAgentMcu::CCliAgentMcu()
    {
        m_IoDevice = NULL;
    }
    
    CCliAgentMcu::~CCliAgentMcu()
    {
        if(m_IoDevice != NULL)
        {
            delete m_IoDevice;
        }
    }

    void CCliAgentMcu::SetIO(CCliAgentMcuIoDevice *IoDevice)
    {
        m_IoDevice = IoDevice;
    }

    CCliAgentMcuIoDevice *CCliAgentMcu::GetIO()
    {
        return m_IoDevice;
    }

}