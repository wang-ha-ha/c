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
//! @brief Line class definition.

#ifndef _CLI_UI_LINE_H_
#define _CLI_UI_LINE_H_

#include "cli/ui.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class CmdLineEdition;


    CLI_NS_BEGIN(ui)

        //! @brief Simple line user interface object.
        class Line : public UI
        {
        public:
            //! @brief Top execution context constructor.
            explicit Line(
                const tk::String& TK_DefaultLine,   //!< Default value.
                const int I_MinLineLength,          //!< Minimum line length required. -1 if not set.
                const int I_MaxLineLength           //!< Maximum line length required. -1 if not set.
                );

            //! @brief Child execution context constructor.
            explicit Line(
                ExecutionContext& CLI_ParentContext,    //!< Parent execution context.
                const tk::String& TK_DefaultLine,       //!< Default value.
                const int I_MinLineLength,              //!< Minimum line length required. -1 if not set.
                const int I_MaxLineLength               //!< Maximum line length required. -1 if not set.
                );

            //! @brief Destructor.
            virtual ~Line(void);

        private:
            //! @brief No default constructor.
            explicit Line(void);
            //! @brief No copy constructor.
            Line(const Line&);
            //! @brief No assignment operator.
            Line& operator=(const Line&);

        public:
            //! @brief Line retrieval.
            //! @return Line entered by the user.
            const tk::String GetLine(void) const;

        protected:
            //! @brief Protected line setter for child classes.
            void SetLine(
                const tk::String& TK_Line,  //!< Line to print out in place of the current line.
                const bool B_NewLine,       //!< Move to a new line.
                const bool B_CleanOnTyping  //!< true when the line should be fully cleaned, false if the line can be modified.
                );
        protected:
            // cli::ui::UI interface implementation.
            virtual void Reset(void);
            virtual void ResetToDefault(void);
        public:
            // Inherit doxygen comments from cli::ExecutionContext interface documentation.
            virtual void OnKey(const KEY E_KeyCode);

        private:
            const tk::String m_tkDefaultLine;   //!< Default line.
            const int m_iMinLineLength;         //!< Minimum line length required.
            const int m_iMaxLineLength;         //!< Maximum line length required.
            CmdLineEdition& m_cliLine;          //!< Line in edition.
            bool m_bResetOnTyping;              //!< Flag indicating whether the current string should be resetted on typing.
        };

    CLI_NS_END(ui)

CLI_NS_END(cli)

#endif // _CLI_UI_LINE_H_
