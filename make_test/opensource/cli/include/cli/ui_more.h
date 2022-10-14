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
//! @brief More class definition.

#ifndef _CLI_UI_MORE_H_
#define _CLI_UI_MORE_H_

#include "cli/ui.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class CmdLineEdition;


    CLI_NS_BEGIN(ui)

        // Forward declarations.
        class Text;
        class TextIterator;


        //! @brief Simple line user interface object.
        class More : public UI
        {
        public:
            //! @brief Top execution context constructor.
            explicit More(
                const unsigned int UI_MaxLines,     //!< Maximum number of lines
                const unsigned int UI_MaxLineLength //!< Maximum length of lines
                );

            //! @brief Child execution context constructor.
            explicit More(
                ExecutionContext& CLI_ParentContext,    //!< Parent execution context.
                const unsigned int UI_MaxLines,         //!< Maximum number of lines
                const unsigned int UI_MaxLineLength     //!< Maximum length of lines
                );

            //! @brief Destructor.
            virtual ~More(void);

        private:
            //! @brief No default constructor.
            explicit More(void);
            //! @brief No copy constructor.
            More(const More&);
            //! @brief No assignment operator.
            More& operator=(const More&);

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
            //! @brief Shows the 'more' message.
            //!
            //! If display is done, the 'more' message is not printed out, and execution is stopped.
            void ShowMoreMessage(void);

            //! @brief Hides the 'more' message.
            //!
            //! Can be called even if the more message is not displayed at this time.
            void HideMoreMessage(void);

            //! @brief Terminates display.
            void Quit(void);

        private:
            const Text& m_uiText;           //!< Member text object.
            TextIterator* m_puiTextIt;      //!< Current text iterator.
            CmdLineEdition& m_cliMoreLine;  //!< Line printed out when the display is paused.
        };

    CLI_NS_END(ui)

CLI_NS_END(cli)

#endif // _CLI_UI_MORE_H_
