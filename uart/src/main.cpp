#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "cli/common.h"
#include "cli/console.h"
#include "cli_agent_client_config.hpp"
#include "cli/io_mux.h"
#include "cli/file_device.h"
#include "cli_agent_args.hpp"
#include "cli_agent_common.hpp"
#include "cli_agent_mcu.hpp"
#include "cli_agent_mcu_io_uart.hpp"
#include "ncurses/curses.h"

#include "log_mx.h"

void InitLog()
{
    maix::T_LogConfig tLogConfig;
    tLogConfig.m_strName = "MXFactory";
    tLogConfig.m_eLevel = maix::MX_LOG_TRACE;
    tLogConfig.m_eType = maix::MX_LOG_CONSOLE;
    tLogConfig.m_strFileName = "";
    tLogConfig.m_eNetType = maix::MX_LOG_TCP;
    tLogConfig.m_strIP = "";
    tLogConfig.m_iPort = 0;
    maix::logInit(tLogConfig);
}

void InitMcu()
{
    maix::T_uMcuIoConfig IoConfig;

    IoConfig.sUartConfig.iBaudRate = 115200;
    IoConfig.sUartConfig.iBits = 8;
    IoConfig.sUartConfig.cParity = 'N';
    IoConfig.sUartConfig.iStop = 1;
    IoConfig.sUartConfig.iInterval = 10; // ms,超过这个时间没有接收到数据代表这一个串口数据报就接收完了；
    strcpy(IoConfig.sUartConfig.strUartPath,UART_PATH);
    
    maix::CCliAgentMcuIoDevice * CliAgentMcuIo = new maix::CCliAgentMcuIoUart();
    CliAgentMcuIo->Configure(IoConfig);
    maix::CCliAgentMcu *CliAgentMcu = maix::CCliAgentMcuFactory::GetInstance(MCU_TYPE);
    CliAgentMcu->SetIO(CliAgentMcuIo);
}

void InitCliDebug()
{
    // static const cli::TraceClass CLI_CLI("CLI_CLI", cli::Help());
    // static const cli::TraceClass CLI_SHELL("CLI_SHELL", cli::Help());
    // static const cli::TraceClass CLI_EXEC_CTX("CLI_EXEC_CTX", cli::Help());
    // static const cli::TraceClass CLI_EXECUTION("CLI_EXECUTION", cli::Help());
    // static const cli::TraceClass CLI_NCURSES_CONSOLE("CLI_NCURSES_CONSOLE", cli::Help());
    // static const cli::TraceClass CLI_TELNET_SERVER("CLI_TELNET_SERVER", cli::Help());
    // class _Trace { public:
    //     explicit _Trace() {
    //         cli::GetTraces().SetStream(cli::OutputDevice::GetStdErr());
    //         cli::GetTraces().Declare(CLI_CLI);        cli::GetTraces().SetFilter(CLI_CLI, false);
    //         cli::GetTraces().Declare(CLI_SHELL);       cli::GetTraces().SetFilter(CLI_SHELL, false);
    //         cli::GetTraces().Declare(CLI_EXEC_CTX);         cli::GetTraces().SetFilter(CLI_EXEC_CTX, false);
    //         cli::GetTraces().Declare(CLI_EXECUTION);         cli::GetTraces().SetFilter(CLI_EXECUTION, false);
    //         cli::GetTraces().Declare(CLI_NCURSES_CONSOLE);         cli::GetTraces().SetFilter(CLI_NCURSES_CONSOLE, false);
    //         cli::GetTraces().Declare(CLI_TELNET_SERVER);         cli::GetTraces().SetFilter(CLI_TELNET_SERVER, false);
    //     }
    //     ~_Trace() {
    //         cli::GetTraces().UnsetStream(cli::OutputDevice::GetStdErr());
    //     }
    // } guard;
}

int main(int I_Args, const char* ARSTR_Args[])
{
    InitLog();
    InitMcu();
    CliAgentCommonInit();

    SampleArgs CliArgs;
    if (! CliArgs.ParseArgs(I_Args, ARSTR_Args)) { 
        return -1;
    }

    if (CliArgs.GetInput() == NULL && CliArgs.IsTelnet() == false) {
        CliArgs.DisplayHelp(ARSTR_Args[0]);
        return -1;
    }

    cli_agent_client CliAgentClient;
    cli::Shell CliShell(CliAgentClient);

    InitCliDebug();

    CliArgs.Execute(CliShell);

    return 0;
}
