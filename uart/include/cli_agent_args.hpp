/*
    Copyright (c) 2006-2018, Alexis Royer, http://alexis.royer.free.fr/CLI

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
          and/or other materials provided with the distribution.
        * Neither the name of the CLI library project nor the names of its contributors may be used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __CLI_AGENT_ARGS_H__
#define __CLI_AGENT_ARGS_H__

// System includes
#include <stdlib.h> // atoi
#include <string.h> // strcmp
#include <string.h>

// Public includes
#include "cli/io_device.h"
#include "cli/console.h"
#include "cli/no_ncurses_console.h"
#include "cli/telnet.h"
#include "cli/file_device.h"
#include "cli/shell.h"
#include "ncurses/curses.h"
#include "cli/common.h"
#include "cli/console.h"
#include "cli/io_mux.h"
#include "cli/file_device.h"

class SampleArgs {
public:
    bool m_bNoExec; int m_iTelnetPort;
    cli::OutputDevice* m_pcliOut; cli::IODevice* m_pcliIn;
public:
    explicit SampleArgs(void) : m_bNoExec(false), m_iTelnetPort(-1), m_pcliOut(NULL), m_pcliIn(NULL) {}
    ~SampleArgs(void) {
        if (m_pcliOut != NULL) { m_pcliOut->FreeInstance(__CALL_INFO__); m_pcliOut = NULL; }
        if (m_pcliIn != NULL) { m_pcliIn->FreeInstance(__CALL_INFO__); m_pcliIn = NULL; }
    }
public:
    const bool ParseArgs(int I_Args, const char* ARSTR_Args[]) {
        // Parse arguments.
        if ((ARSTR_Args == NULL) || (ARSTR_Args[0] == NULL)) { return false; }
        const char *str_OutputFileName = NULL, *str_InputFileName = NULL;
        int is_console = 0;
        for (int i=1; i<I_Args; i++) {
            if (ARSTR_Args[i] == NULL) {}
            else if (strcmp(ARSTR_Args[i], "--help") == 0) { DisplayHelp(ARSTR_Args[0]); }
            else if (strcmp(ARSTR_Args[i], "--console") == 0) { m_iTelnetPort = -1; str_OutputFileName = NULL; str_InputFileName = NULL; is_console = 1;}
            else if (strcmp(ARSTR_Args[i], "--telnet") == 0) { if ((++i < I_Args) && (ARSTR_Args[i] != NULL)) { m_iTelnetPort = atoi(ARSTR_Args[i]); } }
            else if (strcmp(ARSTR_Args[i], "--input") == 0) { if ((++i < I_Args) && (ARSTR_Args[i] != NULL)) { str_InputFileName = ARSTR_Args[i]; } }
            else if (strcmp(ARSTR_Args[i], "--output") == 0) { if ((++i < I_Args) && (ARSTR_Args[i] != NULL)) { str_OutputFileName = ARSTR_Args[i]; } }
            else { cli::OutputDevice::GetStdErr() << "Unknown option '" << ARSTR_Args[i] << "'" << cli::endl; DisplayHelp(ARSTR_Args[0]); return false; }
        }

        // Create devices.
        if (m_bNoExec) {}
        else if (m_iTelnetPort > 0) {
            // cli::OutputDevice::GetStdOut() << "m_iTelnetPort:" << m_iTelnetPort << cli::endl;
        }
        else if (str_InputFileName != NULL) {
            if (str_OutputFileName != NULL) {
                m_pcliOut = new cli::OutputFileDevice(str_OutputFileName, true);
            } else {
                m_pcliOut = new cli::Console(true);
            }

            cli::InputFileDevice* const pcli_FileDevice = new cli::InputFileDevice(str_InputFileName, *m_pcliOut, true);
            pcli_FileDevice->EnableSpecialCharacters(true);
            m_pcliIn = pcli_FileDevice;
        } else if (is_console == 1) {
            m_pcliOut = m_pcliIn = new cli::Console(true);
        }
        if (m_pcliOut) {
            // cli::OutputDevice::GetStdOut() << "Using " << m_pcliOut->GetDebugName() << " for output" << cli::endl;
            m_pcliOut->UseInstance(__CALL_INFO__);
        }
        if (m_pcliIn) {
            // cli::OutputDevice::GetStdOut() << "Using " << m_pcliIn->GetDebugName() << " for input" << cli::endl;
            m_pcliIn->UseInstance(__CALL_INFO__);
        }

        return true;
    }
    const bool IsNoExec(void) const { return m_bNoExec; }
    const bool IsConsole(void) const { return ((m_iTelnetPort < 0) && (m_pcliIn != NULL)); }
    const bool IsTelnet(void) const { return (m_iTelnetPort > 0); }
    cli::IODevice* const GetInput(void) { return m_pcliIn; }
    cli::OutputDevice* const GetOutput(void) { return m_pcliOut; }

    void Execute(cli::Shell& CLI_Shell) const {
        if (m_bNoExec) {}
        else if (m_iTelnetPort > 0) {
            class TestTelnetServer : public cli::TelnetServer {
            private:
                cli::Shell& m_cliShell;
            public:
                TestTelnetServer(cli::Shell& CLI_Shell, const unsigned long UL_TcpPort,bool *B_StreamEnables) : TelnetServer(20, UL_TcpPort, cli::ResourceString::LANG_EN,B_StreamEnables), m_cliShell(CLI_Shell){} // because the CLI is allocated once only, allow only one client.
                virtual cli::ExecutionContext* const OnNewConnection(const cli::TelnetConnection& CLI_NewConnection) { return & m_cliShell; }
                virtual void OnCloseConnection(const cli::TelnetConnection& CLI_ClosedConnection, cli::ExecutionContext* const PCLI_Context) {}
            };

            // cli::OutputDevice::GetStdOut() << "Running telnet server on port " << m_iTelnetPort << cli::endl;
            bool StreamEnabled[cli::STREAM_TYPES_COUNT];

            StreamEnabled[cli::WELCOME_STREAM] = false;
            StreamEnabled[cli::PROMPT_STREAM] = false;
            StreamEnabled[cli::ECHO_STREAM] = false;
            StreamEnabled[cli::OUTPUT_STREAM] = true;
            StreamEnabled[cli::ERROR_STREAM] = true;


            TestTelnetServer cli_Server(CLI_Shell, m_iTelnetPort,StreamEnabled);
            cli_Server.StartServer();
        }
        else if (m_pcliIn != NULL) {
            std::string str_InFile;
            if (const cli::InputFileDevice* const pcli_In = dynamic_cast<const cli::InputFileDevice*>(m_pcliIn)) {
                str_InFile = pcli_In->GetFileName();
            }
            // cli::OutputDevice::GetStdOut() << "Regular execution with device "  << str_InFile.c_str() << cli::endl;
            // cli::OutputDevice::GetStdOut() << "Regular execution with device " << m_pcliIn->GetDebugName() << cli::endl;
            if(str_InFile.compare("") == 0){
                CLI_Shell.Run(*m_pcliIn);
            } 
            else {
                cli::No_Ncurses_Console cli_Console(true, str_InFile.c_str());
                bool StreamEnabled[cli::STREAM_TYPES_COUNT];

                StreamEnabled[cli::WELCOME_STREAM] = false;
                StreamEnabled[cli::PROMPT_STREAM] = false;
                StreamEnabled[cli::ECHO_STREAM] = false;
                StreamEnabled[cli::OUTPUT_STREAM] = true;
                StreamEnabled[cli::ERROR_STREAM] = true;

                CLI_Shell.Run(cli_Console,StreamEnabled);
            }
        }
    }

    void DisplayHelp(const char* const STR_ProgramName) {
        cli::OutputDevice::GetStdOut() << "USAGE" << cli::endl;
        cli::OutputDevice::GetStdOut() << "   " << STR_ProgramName << " --help" << cli::endl;
        cli::OutputDevice::GetStdOut() << "       Display this help." << cli::endl;
        cli::OutputDevice::GetStdOut() << "   " << STR_ProgramName << " [--console]" << cli::endl;
        cli::OutputDevice::GetStdOut() << "       Interactive mode." << cli::endl;
        cli::OutputDevice::GetStdOut() << "   " << STR_ProgramName << " --telnet <port>" << cli::endl;
        cli::OutputDevice::GetStdOut() << "       Telnet mode." << cli::endl;
        cli::OutputDevice::GetStdOut() << "   " << STR_ProgramName << " [--input <input file> [--output <output file>]]" << cli::endl;
        cli::OutputDevice::GetStdOut() << "       Input from file, output to file or standard output." << cli::endl;
        m_bNoExec = true;
    }
};

#endif // __CLI_AGENT_ARGS_H__
