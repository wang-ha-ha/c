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
//! @brief Traces menu definition.

#ifndef _CLI_TRACES_MENU_H_
#define _CLI_TRACES_MENU_H_

#include "cli/namespace.h"
#include "cli/menu.h"
#include "cli/traces.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class ParamString;

    //! @brief Traces menu definition.
    class TracesMenu : public Menu
    {
    public:
        //! @brief Default constructor.
        explicit TracesMenu(void);

        //! @brief Destructor.
        virtual ~TracesMenu(void);

    private:
        //! @brief No copy constructor.
        TracesMenu(const TracesMenu&);
        //! @brief No assignment operator.
        TracesMenu& operator=(const TracesMenu&);

    public:
        // Inherit doxygen comments from cli::Element documentation.
        virtual void SetCli(Cli& CLI_Cli);
        // Inherit doxygen comments from cli::Menu documentation.
        virtual const bool ExecuteReserved(const CommandLine& CLI_CmdLine) const;
        // Inherit doxygen comments from cli::Menu documentation.
        virtual const bool Execute(const CommandLine& CLI_CmdLine) const;

    private:
        //! @brief Show all classes.
        void ShowAllClasses(void) const;

        //! @brief Show current filter.
        void ShowCurrentFilter(void) const;

        //! @brief Display a class list.
        void DisplayClassList(
            const TraceClass::List& Q_Classes   //!< Class list to display.
            ) const;

        //! @brief Filter modification.
        void SetFilter(
            const char* const STR_ClassName,    //!< Filter name.
            const bool B_Show                   //!< Show flag.
            ) const;

        //! @brief Global filter modification.
        void SetAllFilter(
            const bool B_Show           //!< Show flag.
            ) const;

    private:
        //! @brief 'show' node access.
        const Keyword& GetShowNode(void) const;
        //! @brief 'show filter' node access.
        const Keyword& GetShowFilterNode(void) const;
        //! @brief 'show classes' node access.
        const Keyword& GetShowClassesNode(void) const;
        //! @brief 'no' node access.
        const Keyword& GetNoNode(void) const;
        //! @brief 'trace' node access.
        const Keyword& GetTraceNode(void) const;
        //! @brief 'trace filter' node access.
        const Keyword& GetFilterNode(void) const;
        //! @brief 'trace filter <filter>' node access.
        const ParamString& GetFilterParam(void) const;
        //! @brief 'trace all' node access.
        const Keyword& GetAllFilterNode(void) const;

    private:
        //! 'show' node.
        Keyword* m_pcliShowNode;
        //! 'show filter' node.
        Keyword* m_pcliShowFilterNode;
        //! 'show classes' node.
        Keyword* m_pcliShowClassesNode;

        //! 'no' node.
        Keyword* m_pcliNoNode;
        //! 'trace' node.
        Keyword* m_pcliTraceNode;
        //! 'trace filter' node.
        Keyword* m_pcliTraceFilterNode;
        //! 'trace filter <filter>' node.
        ParamString* m_pcliFilterParam;
        //! 'trace all' node.
        Keyword* m_pcliAllFilterNode;
    };

CLI_NS_END(cli)

#endif // _CLI_TRACES_MENU_H_
