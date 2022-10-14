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
//! @brief Menu and MenuRef classes definition.

#ifndef _CLI_MENU_H_
#define _CLI_MENU_H_

#include "cli/namespace.h"
#include "cli/syntax_node.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class Cli;
    class Console;
    class CommandLine;
    class Keyword;
    class Help;


    //! @brief Menu definition.
    class Menu : public SyntaxNode
    {
    public:
        //! @brief Constructor.
        explicit Menu(
            const char* const STR_Name,     //!< Menu name.
            const Help& CLI_Help            //!< Corresponding help.
            );

        //! @brief Destructor.
        virtual ~Menu(void);

    private:
        //! @brief No default constructor.
        explicit Menu(void);
        //! @brief No copy constructor.
        Menu(const Menu&);
        //! @brief No assignment operator.
        Menu& operator=(const Menu&);

    public:
        //! @brief Menu name access.
        //! @return Name of the menu.
        const tk::String GetName(void) const;

    public:
        // Inherit doxygen comments from cli::Element documentation.
        virtual void SetCli(Cli& CLI_Cli);
        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief Reserved commands execution.
        //! @param CLI_CommandLine Command line to execute.
        //! @return true if the command has found an execution code, false otherwise.
        virtual const bool ExecuteReserved(const CommandLine& CLI_CommandLine) const;
        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief User-defined commands execution.
        //! @param CLI_CommandLine Command line to execute.
        //! @return true if the command has found an execution code, false otherwise.
        virtual const bool Execute(const CommandLine& CLI_CommandLine) const;
        //! @brief Handler on menu exit.
        virtual void OnExit(void) const;
        //! @brief Handler on prompt display.
        //! @return Prompt to display. Empty string for default.
        virtual const tk::String OnPrompt(void) const;

    private:
        //! Help node.
        Keyword* m_pcliHelp;
        //! Exit node.
        Keyword* m_pcliExit;
        //! Quit node.
        Keyword* m_pcliQuit;
        //! Print working menu node.
        Keyword* m_pcliPwm;
        //! Clean screen node.
        Keyword* m_pcliCls;
    };


    //! @brief Menu reference element.
    class MenuRef : public Element
    {
    public:
        //! @brief Constructor.
        explicit MenuRef(
            const Menu& CLI_Menu        //!< Referenced menu.
            );

        //! @brief Destructor.
        virtual ~MenuRef(void);

    private:
        //! @brief No default constructor.
        explicit MenuRef(void);
        //! @brief No copy constructor.
        MenuRef(const MenuRef&);
        //! @brief No assignment operator.
        MenuRef& operator=(const MenuRef&);

    public:
        //! @brief Target menu accessor.
        //! @return Target menu reference.
        const Menu& GetMenu(void) const;

    private:
        //! Referenced menu.
        const Menu* const m_pcliMenu;
    };

CLI_NS_END(cli)

#endif // _CLI_MENU_H_
