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


//! @file
//! @author Alexis Royer
//! @brief ConfigMenu class definition.

#ifndef _CLI_CONFIG_MENU_H_
#define _CLI_CONFIG_MENU_H_

#include "cli/namespace.h"
#include "cli/menu.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class Keyword;
    class OutputDevice;


    //! @brief CLI configuration menu definition.
    class ConfigMenu : public Menu
    {
    public:
        //! @brief Default constructor.
        explicit ConfigMenu(void);

        //! @brief Destructor.
        virtual ~ConfigMenu(void);

    private:
        //! @brief No copy constructor.
        ConfigMenu(const ConfigMenu&);
        //! @brief No assignment operator.
        ConfigMenu& operator=(const ConfigMenu&);

    public:
        // Inherit doxygen comments from cli::Element documentation.
        virtual void SetCli(Cli& CLI_Cli);
        // Inherit doxygen comments from cli::Menu documentation.
        virtual const bool ExecuteReserved(const CommandLine& CLI_CmdLine) const;
        // Inherit doxygen comments from cli::Menu documentation.
        virtual const bool Execute(const CommandLine& CLI_CmdLine) const;

    private:
        //! @brief 'show version' command.
        void ShowVersion(void) const;
        //! @brief 'show author' command.
        void ShowAuthor(void) const;
        //! @brief 'show license' command.
        void ShowLicense(void) const;

        //! @brief 'echo on' command.
        void EchoOn(void) const;

        //! @brief 'echo off' command.
        void EchoOff(void) const;

        //! @brief 'beep on' command.
        void BeepOn(void) const;

        //! @brief 'beep off' command.
        void BeepOff(void) const;

        //! @brief Language setting.
        void SetLang(const ResourceString::LANG E_Lang) const;

        #ifdef _DEBUG
        //! @brief Check input/output device.
        void CheckIODevice(void) const;
        //! @brief Check output character device.
        void CheckOutChar(void) const;
        #endif

    private:
        //! @brief Show node access (read-only).
        const Keyword& GetShowNode(void) const;
        //! @brief Show version node access (read-only).
        const Keyword& GetShowVersionNode(void) const;
        //! @brief Show author node access (read-only).
        const Keyword& GetShowAuthorNode(void) const;
        //! @brief Show license node access (read-only).
        const Keyword& GetShowLicenseNode(void) const;

        //! @brief Echo node access (read-only).
        const Keyword& GetEchoNode(void) const;
        //! @brief Echo on node access (read-only).
        const Keyword& GetEchoOnNode(void) const;
        //! @brief Echo off node access (read-only).
        const Keyword& GetEchoOffNode(void) const;

        //! @brief Beep node access (read-only).
        const Keyword& GetBeepNode(void) const;
        //! @brief Beep on node access (read-only).
        const Keyword& GetBeepOnNode(void) const;
        //! @brief Beep off node access (read-only).
        const Keyword& GetBeepOffNode(void) const;

        //! @brief Language node access (read-only).
        const Keyword& GetLangNode(void) const;
        //! @brief English language node access (read-only).
        const Keyword& GetEnglishLangNode(void) const;
        //! @brief French language node access (read-only).
        const Keyword& GetFrenchLangNode(void) const;

        #ifdef _DEBUG
        //! @brief Check device node (read-only).
        const Keyword& GetCheckNode(void) const;
        //! @brief Check IO device node (read-only).
        const Keyword& GetCheckIONode(void) const;
        //! @brief Check output character node (read-only).
        const Keyword& GetOutCharNode(void) const;
        #endif

    private:
        //! Show node.
        Keyword* m_pcliShow;
        //! Show version node.
        Keyword* m_pcliShowVersion;
        //! Show author node.
        Keyword* m_pcliShowAuthor;
        //! Show license node.
        Keyword* m_pcliShowLicense;

        //! Echo node.
        Keyword* m_pcliEcho;
        //! Echo on node.
        Keyword* m_pcliEchoOn;
        //! Echo off node.
        Keyword* m_pcliEchoOff;

        //! Beep node.
        Keyword* m_pcliBeep;
        //! Beep on node.
        Keyword* m_pcliBeepOn;
        //! Beep off node.
        Keyword* m_pcliBeepOff;

        //! Lang node.
        Keyword* m_pcliLang;
        //! English language node.
        Keyword* m_pcliEnglishLang;
        //! French language node.
        Keyword* m_pcliFrenchLang;

        #ifdef _DEBUG
        //! Check node.
        Keyword* m_pcliCheck;
        //! Check IO device node.
        Keyword* m_pcliCheckIO;
        //! Check output character node.
        Keyword* m_pcliOutChar;
        #endif
    };

CLI_NS_END(cli)

#endif // _CLI_CONFIG_MENU_H_
