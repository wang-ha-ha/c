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
//! @brief YesNo class definition.

#ifndef _CLI_UI_YESNO_H_
#define _CLI_UI_YESNO_H_

#include "cli/ui_choice.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        //! @brief Yes/No question user interface object.
        class YesNo : public Choice
        {
        public:
            //! @brief Top execution context constructor.
            explicit YesNo(
                const bool B_DefaultAnswer              //!< Default answer: true for 'yes', false for 'no'.
                );

            //! @brief Child execution context constructor.
            explicit YesNo(
                ExecutionContext& CLI_ParentContext,    //!< Parent execution context.
                const bool B_DefaultAnswer              //!< Default answer: true for 'yes', false for 'no'.
                );

            //! @brief Destructor.
            virtual ~YesNo(void);

        private:
            //! @brief No default constructor.
            explicit YesNo(void);
            //! @brief No copy constructor.
            YesNo(const YesNo&);
            //! @brief No assignment operator.
            YesNo& operator=(const YesNo&);

        public:
            //! @brief Yes/No answer retrieval.
            //! @return true for 'yes', false for 'no'.
            const bool GetYesNo(void) const;

        public:
            // Inherit doxygen comments from cli::ExecutionContext interface documentation.
            virtual void OnKey(const KEY E_KeyCode);
        };

    CLI_NS_END(ui)

CLI_NS_END(cli)

#endif // _CLI_UI_YESNO_H_
