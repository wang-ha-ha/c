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
//! @brief Choice class definition.

#ifndef _CLI_UI_CHOICE_H_
#define _CLI_UI_CHOICE_H_

#include "cli/ui_line.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        //! @brief Choice user interface class.
        class Choice : public Line
        {
        public:
            //! @brief Top execution context constructor.
            explicit Choice(
                const int I_DefaultChoice,                  //!< Index in the input queue corresponding to the default answer.
                const tk::Queue<ResourceString>& TK_Choices //!< Input choice list.
                );

            //! @brief Child execution context constructor.
            explicit Choice(
                ExecutionContext& CLI_ParentContext,        //!< Parent execution context.
                const int I_DefaultChoice,                  //!< Index in the input queue corresponding to the default answer.
                const tk::Queue<ResourceString>& TK_Choices //!< Input choice list.
                );

            //! @brief Destructor.
            virtual ~Choice(void);

        private:
            //! @brief No default constructor.
            explicit Choice(void);
            //! @brief No copy constructor.
            Choice(const Choice&);
            //! @brief No assignment operator.
            Choice& operator=(const Choice&);

        public:
            //! @brief Choice retrieval.
            //! @return Index in the input queue corresponding to the answer entered by the user.
            //!         -1 if no matching choice found.
            const int GetChoice(void) const;

            //! @brief Choice retrieval in its string form.
            //! @return String of the choice.
            //!         Empty string if no matching choice found.
            const ResourceString GetstrChoice(void) const;

        protected:
            // cli::ui::UI interface implementation.
            virtual void ResetToDefault(void);
        public:
            // Inherit doxygen comments from cli::ExecutionContext interface documentation.
            virtual void OnKey(const KEY E_KeyCode);
        private:
            //! @brief Moves the current choice on arrow keys.
            void MoveChoice(
                const int I_Way,                //!< 1 for DOWN keys, -1 for UP keys.
                const unsigned int UI_Increment //!< Increment of movement.
                );

        private:
            const int m_iDefaultChoice;                     //!< Default choice.
            const tk::Queue<ResourceString> m_tkChoices;    //!< Input choice list.
            ResourceString::LANG m_eLang;                   //!< Language setting memorization so that post-execution calls do not need a shell instance reference anymore.
        };

    CLI_NS_END(ui)

CLI_NS_END(cli)

#endif // _CLI_UI_CHOICE_H_
