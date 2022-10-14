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
//! @brief CommandLine class definition.

#ifndef _CLI_COMMAND_LINE_H_
#define _CLI_COMMAND_LINE_H_

#include "cli/namespace.h"
#include "cli/object.h"
#include "cli/element.h"
#include "cli/resource_string.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class Menu;


    //! @brief Command line parsing.
    //!
    //! Parses a command line in its string form,
    //! and returns the command line in its Element collection form after analysis.
    class CommandLine : public Object
    {
    public:
        //! @brief Default constructor.
        explicit CommandLine(void);

        //! @brief Destructor.
        virtual ~CommandLine(void);

    private:
        //! @brief No copy constructor.
        CommandLine(const CommandLine&);
        //! @brief No assignment operator.
        CommandLine& operator=(const CommandLine&);

    public:
        //! @brief Parse and analysis invocation.
        //! @return true for success, false otherwise.
        //!
        //! If this method succeeds,
        //! the result is stored in the object itself,
        //! and can be accessed through the following public methods.
        const bool Parse(
            const Menu& CLI_Menu,           //!< Current menu.
            const tk::String& STR_Line,    //!< Input command line in its string form.
            const bool B_Execution          //!< Flag set when parsing is done for execution.
                                            //!< Implicitely say completion otherwise.
            );

        //! @brief Last element accessor.
        //! @return Last element reference.
        const Element& GetLastElement(void) const;

        //! @brief Last word (for completion).
        //! @return The last word if any, NULL when no last word.
        //! @note After parsing, the buffer is valid as far as the command line object is valid.
        //! 
        //! When used for completion, this object does not analyse the last word and just stores it.
        //! This method retrieves this last word.
        const char* const GetLastWord(void) const;

        //! @brief Number of backspaces for completion.
        //! @return Number of backspaces.
        //!
        //! Number of backspaces in order to erase the last word of the line.
        const int GetNumBackspacesForCompletion(void) const;

        //! @brief Last error.
        //! @return Last error resource string.
        //!
        //! This resource is cleared on every parse.
        const ResourceString& GetLastError(void) const;

    private:
        //! @brief Splits a command line in words.
        //! @return true for success, false otherwise.
        const bool Split(
            const tk::String& STR_Line,            //!< Input command line in its string form.
            tk::Queue<tk::String>& Q_Words,         //!< Word list.
            int& I_LastWordPosition                 //!< Last word position in the list.
            );

        //! @brief Element referencing.
        //! @return The CommandLine object itself.
        //!
        //! Pushes an element reference during analysis.
        CommandLine& AddElement(
            const Element* const PCLI_Element       //!< New element reference.
            );

    private:
        //! Collection of corresponding element references for the command line.
        Element::List m_cliElements;
        //! List of element references to destroy automatically in the CommandLine destructor.
        Element::List m_cliAutoDelete;
        //! Current menu reference.
        const Element* m_pcliMenu;
        //! Last word used for completion.
        tk::String m_strLastWord;
        //! Last word validity flag.
        bool m_bLastWordValid;
        //! Number of backspaces for completion.
        int m_iNumBackspacesForCompletion;
        //! Last error.
        ResourceString m_cliError;

    private:
        // In order to allow access to private members for iteration.
        friend class CommandLineIterator;
    };

    //! @brief Command line iteration.
    //!
    //! Scans the results of a command line analysis described in the CommandLine class.
    class CommandLineIterator : public Object
    {
    public:
        //! @brief Regular constructor.
        explicit CommandLineIterator(
            const CommandLine& CLI_CmdLine  //!< Command line objet to iterate.
            );

        //! @brief Destructor.
        virtual ~CommandLineIterator(void);

    private:
        //! @brief No default constructor.
        explicit CommandLineIterator(void);
        //! @brief No copy constructor.
        CommandLineIterator(const CommandLineIterator&);
        //! @brief No assignment operator.
        CommandLineIterator& operator=(const CommandLineIterator&);

    public:
        //! @brief Iteration.
        //! @return true: Iteration succeeded.
        //! @return false: Iteration failed.
        const bool StepIt(void);

        //! @brief Checks whether the element corresponds to the current element.
        //! @return true if the element matches, false otherwise.
        const bool operator==(
            const Element& CLI_Element      //!< Element to check.
            ) const;

        //! @brief Current element accessor.
        //! @return Current element reference.
        const Element* const operator*(void) const;

    private:
        //! Command line object to iterate.
        const CommandLine& m_cliCmdLine;

        //! Current position.
        Element::List::Iterator m_cliIterator;

        //! Current element.
        const Element* m_pcliCurrentElement;
    };

CLI_NS_END(cli)

#endif // _CLI_COMMAND_LINE_H_

