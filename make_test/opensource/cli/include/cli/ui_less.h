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
//! @brief Less class definition.

#ifndef _CLI_UI_LESS_H_
#define _CLI_UI_LESS_H_

#include "cli/ui.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class CmdLineEdition;


    CLI_NS_BEGIN(ui)

        // Forward declarations.
        class Text;
        class TextIterator;


        //! @brief Simple line user interface object.
        class Less : public UI
        {
        public:
            //! @brief Top execution context constructor.
            explicit Less(
                const unsigned int UI_MaxLines,     //!< Maximum number of lines
                const unsigned int UI_MaxLineLength //!< Maximum length of lines
                );

            //! @brief Child execution context constructor.
            explicit Less(
                ExecutionContext& CLI_ParentContext,    //!< Parent execution context.
                const unsigned int UI_MaxLines,     //!< Maximum number of lines
                const unsigned int UI_MaxLineLength //!< Maximum length of lines
                );

            //! @brief Destructor.
            virtual ~Less(void);

        private:
            //! @brief No default constructor.
            explicit Less(void);
            //! @brief No copy constructor.
            Less(const Less&);
            //! @brief No assignment operator.
            Less& operator=(const Less&);

        public:
            //! @brief Text member accessor.
            //! @return Text member reference.
            const OutputDevice& GetText(void);

        // cli::ui::UI interface implementation.
        protected:
            virtual void Reset(void);
            virtual void ResetToDefault(void);
        public:
            // Inherit doxygen comments from cli::ExecutionContext interface documentation.
            virtual void OnKey(const KEY E_KeyCode);

        private:
            //! @brief Print out the current screen.
            void PrintScreen(void);

            //! @brief Terminates display.
            void Quit(void);

        private:
            const Text& m_uiText;           //!< Member text object.
            TextIterator* m_puiTextIt;      //!< Current text iterator.
            CmdLineEdition& m_cliLessLine;  //!< Status line of the 'less' display.
        };

    CLI_NS_END(ui)

CLI_NS_END(cli)

#endif // _CLI_UI_LESS_H_
