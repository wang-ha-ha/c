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
//! @brief Shell class definition.

#ifndef _CLI_SHELL_H_
#define _CLI_SHELL_H_

#include "cli/namespace.h"
#include "cli/exec_context.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class Cli;
    class Menu;
    class Element;
    class CmdLineEdition;
    class CmdLineHistory;


    //! @brief Shell management.
    class Shell : public ExecutionContext
    {
    private:
        //! @brief Object initialization.
        void InitObject(
            const Cli& CLI_Cli                      //!< CLI reference.
            );

    public:
        //! @brief Top execution context constructor.
        explicit Shell(
            const Cli& CLI_Cli                      //!< CLI reference.
            );

        //! @brief Child execution context constructor.
        explicit Shell(
            ExecutionContext& CLI_ParentContext,    //!< Parent execution context.
            const Cli& CLI_Cli                      //!< CLI reference.
            );

        //! @brief Destructor.
        virtual ~Shell(void);

    private:
        //! @brief No default constructor.
        explicit Shell(void);
        //! @brief No copy constructor.
        Shell(const Shell&);
        //! @brief No assignment operator.
        Shell& operator=(const Shell&);

    public:
        //! @brief Cli accessor.
        //! @return Cli instance the shell is running onto.
        const Cli& GetCli(void) const;

        //! @brief Welcome message setting.
        void SetWelcomeMessage(
            const ResourceString& CLI_WelcomeMessage    //!< Welcome message.
                                                        //!< When an empty string is given, the default
                                                        //!< welcome message is restored.
            );
        //! @brief Bye message setting.
        void SetByeMessage(
            const ResourceString& CLI_ByeMessage        //!< Bye message.
                                                        //!< When an empty string is given, the default
                                                        //!< bye message is restored.
            );
        //! @brief Prompt message positionning.
        void SetPrompt(
            const ResourceString& CLI_Prompt            //!< Prompt string.
                                                        //!< When an empty string is given, the default
                                                        //!< prompt (depending on the current menu)
                                                        //!< is restored.
            );

    public:
        //! @brief Help margin accessor.
        //! @return Number of spaces for the help margin.
        const unsigned int GetHelpMargin(void) const;

        //! @brief Help offset accessor.
        //! @return Number of spaces for the help offset.
        const unsigned int GetHelpOffset(void) const;

    private:
        //! @brief Prints the welcome message.
        void PromptWelcomeMessage(void) const;
        //! @brief Prints the bye message.
        void PromptByeMessage(void) const;
        //! @brief Prints the prompt message, indicating the current menu.
        void PromptMenu(void) const;
        //! @brief Error display.
        void PrintError(
            const ResourceString& CLI_Location,     //!< Error location
            const ResourceString& CLI_ErrorMessage  //!< Error message
            ) const;

    public:
        //! @brief Current menu retrieval.
        //! @return Reference of the current menu. NULL when I_MenuIndex is out of bounds.
        const Menu* const GetCurrentMenu(
            const int I_MenuIndex       //!< Index of the menu in the stack.
                                        //!< 0: root menu (bottom of the stack).
                                        //!< 1: menu stacked over the root menu.
                                        //!< 2: menu stacked over again...
                                        //!< -1: current menu (top of the stack)
            ) const;

        //! @brief Enters a menu.
        void EnterMenu(
            const Menu& CLI_Menu,       //!< Menu to enter.
            const bool B_PromptMenu     //!< true if the menu should be (re)prompted.
                                        //!< Basically false when executed within the context of a command processing,
                                        //!< true when executed from other contexts.
            );

        //! @brief Exits the current menu.
        void ExitMenu(
            const bool B_PromptMenu     //!< true if the menu should be (re)prompted.
                                        //!< Basically false when executed within the context of a command processing,
                                        //!< true when executed from other contexts.
            );

        //! @brief Terminates the shell.
        //! @warning Not thread safe. Implement a non blocking device if your CLI execution should interact with external events.
        void Quit(void);

        //! @brief Displays help depending on the context of the current line.
        void DisplayHelp(void);

        //! @brief Prints the working menu.
        void PrintWorkingMenu(void);

        //! @brief Clean screen shortcut.
        void CleanScreen(
            const bool B_PromptMenu     //!< true if the menu should be (re)prompted.
                                        //!< Basically false when executed within the context of a command processing,
                                        //!< true when executed from other contexts.
            );

    protected:
        // Inherit doxygen comments from cli::ExecutionContext interface documentation.
        virtual const bool OnStartExecution(void);
        // Inherit doxygen comments from cli::ExecutionContext interface documentation.
        virtual void OnKey(const KEY E_KeyCode);
        // Inherit doxygen comments from cli::ExecutionContext interface documentation.
        virtual const bool OnStopExecution(void);

    private:
        //! @brief Called when a printable character comes up from the input device.
        void OnPrintableChar(const KEY E_KeyCode);
        //! @brief Called when using the 'home'/'begin' key.
        void OnKeyBegin(void);
        //! @brief Called when using the 'end' key.
        void OnKeyEnd(void);
        //! @brief Called when using the left arrow.
        void OnKeyLeft(void);
        //! @brief Called when using the right arrow.
        void OnKeyRight(void);
        //! @brief Called when a backspace comes up from the input device.
        void OnBackspace(void);
        //! @brief Called when a suppr comes up from the input device.
        void OnSuppr(void);
        //! @brief Called when an escape character comes up from the input device.
        void OnEscape(void);
        //! @brief Called when an exit character (CTRL+C) comes up from the input device.
        void OnExit(
            const bool B_PromptMenu     //!< true if the menu should be (re)prompted.
                                        //!< Basically false when executed within the context of a command processing,
                                        //!< true when executed from other contexts.
            );
        //! @brief Called when an help character ('?' or TAB) comes up from the input device.
        //!
        //! This method manages both help and completion.
        void OnHelp(
            const bool B_PromptMenu,    //!< true if the menu should be (re)prompted.
                                        //!< Basically false when executed within the context of a command processing,
                                        //!< true when executed from other contexts.
            const bool B_Completion     //!< true it completion should be performed.
            );
        //! @brief Called when an execute character (ENTER) comes up from the input device.
        void OnExecute(void);
        //! @brief Moves in history.
        void OnHistory(
            const int I_Navigation      //!< Number of steps in history navigation.
                                        //!< Positive values mean navigating to older command lines.
                                        //!< Negative values mean navigating to newer command lines.
            );

    private:
        //! @brief Prints an help line for a given element.
        void PrintHelp(
            const Element& CLI_Element  //!< Element to print the help for.
            );

    private:
        //! CLI reference.
        const Cli& m_cliCli;
        //! Welcome message.
        ResourceString m_cliWelcomeMessage;
        //! Bye message.
        ResourceString m_cliByeMessage;
        //! Non-default Prompt.
        ResourceString m_cliNoDefaultPrompt;
        //! Menu stack.
        tk::Queue<const Menu*> m_qMenus;
        //! Current line.
        CmdLineEdition& m_cliCmdLine;
        //! History.
        CmdLineHistory& m_cliHistory;
    };

CLI_NS_END(cli)

#endif // _CLI_SHELL_H_

