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
//! @brief CmdLineHistory class definition.

#ifndef _CLI_CMD_LINE_HISTORY_H_
#define _CLI_CMD_LINE_HISTORY_H_

#include "cli/namespace.h"
#include "cli/object.h"
#include "cli/tk.h"

#include "command_line_edition.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class CmdLineEdition;


    //! @brief Command line history objet.
    class CmdLineHistory : public Object
    {
    public:
        //! @brief Default constructor.
        explicit CmdLineHistory(
            const unsigned int UI_StackSize //!< Number of history lines the stack should manage.
            );

        //! @brief Destructor.
        virtual ~CmdLineHistory(void);

    private:
        //! @brief No copy constructor.
        CmdLineHistory(const CmdLineHistory&);
        //! @brief No assignment operator.
        CmdLineHistory& operator=(const CmdLineHistory&);

    public:
        //! @brief Pushes a command line in the history stack.
        //! @return true for success, false for failure.
        const bool Push(
            const CmdLineEdition& CLI_Line  //!< Command line to push.
            );

        //! @brief Cleares all history from the stack.
        //! @return true for success, false for failure.
        const bool Clear(void);

    public:
        //! @brief History line retrieval.
        //! @return The expected command line.
        const CmdLineEdition& GetLine(
            const unsigned int UI_BackwardIndex //!< Backward index in history lines.
                                                //!< 0 means the current line.
            ) const;

        //! @brief History line count retrieval.
        //! @return The number of history lines stacked + 1 for the current one.
        const unsigned int GetCount(void) const;

    public:
        //! @brief Saves the current line.
        //! @return true for success, false otherwise.
        const bool SaveCurrentLine(
            const CmdLineEdition& CLI_CurrentLine   //!< Current line.
            );

        //! @brief History line navigation.
        //! @return true if all the navigation expected has been executed,
        //!         false otherwise.
        const bool Navigate(
            CmdLineEdition& CLI_CmdLine,    //!< Current command line edition object.
            const OutputDevice& CLI_Stream, //!< Output stream that should be used to display command lines.
            const int I_Navigation          //!< Number of steps in history navigation.
                                            //!< Positive values mean navigating to older command lines.
                                            //!< Negative values mean navigating to newer command lines.
            );

        //! @brief History index retrieval.
        //! @return The index of the current navigation position.
        const unsigned int GetNavigationIndex(void) const;

        //! @brief Navigation memory enabling.
        //!
        //! As far as the navigation memory is on, the navigation keeps going on.
        //! When something happens while the navigation memory is off, the navigation starts from the beginning.
        void EnableNavigationMemory(
            const bool B_NavigationMemory   //!< true for enabling, false otherwise.
            );

    private:
        //! Stack size.
        const unsigned int m_uiStackSize;
        //! History.
        tk::Queue<CmdLineEdition> m_qHistory;
        //! History index.
        unsigned int m_uiNavigationIndex;
        //! Navigation mode.
        bool m_bNavigation;
    };

CLI_NS_END(cli)

#endif // _CLI_CMD_LINE_HISTORY_H_
