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
//! @brief Float class definition.

#ifndef _CLI_UI_FLOAT_H_
#define _CLI_UI_FLOAT_H_

#include "cli/ui_line.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        //! @brief Float user interface class.
        class Float : public Line
        {
        public:
            //! @brief Top execution context constructor.
            explicit Float(
                const double D_DefaultValue,            //!< Default value.
                const double D_MinValue,                //!< Minimum value.
                const double D_MaxValue                 //!< Maximum value.
                );

            //! @brief Child execution context constructor.
            explicit Float(
                ExecutionContext& CLI_ParentContext,    //!< Parent execution context.
                const double D_DefaultValue,            //!< Default value.
                const double D_MinValue,                //!< Minimum value.
                const double D_MaxValue                 //!< Maximum value.
                );

            //! @brief Destructor.
            virtual ~Float(void);

        private:
            //! @brief No default constructor.
            explicit Float(void);
            //! @brief No copy constructor.
            Float(const Float&);
            //! @brief No assignment operator.
            Float& operator=(const Float&);

        public:
            //! @brief Float retrieval.
            //! @return Float value entered by the user.
            const double GetFloat(void) const;

        protected:
            // cli::ui::UI interface implementation.
            virtual void ResetToDefault(void);
        public:
            // Inherit doxygen comments from cli::ExecutionContext interface documentation.
            virtual void OnKey(const KEY E_KeyCode);

        private:
            const double m_dDefaultValue;   //!< Default value.
            const double m_dMinValue;       //!< Minimum value.
            const double m_dMaxValue;       //!< Maximum value.
        };

    CLI_NS_END(ui)

CLI_NS_END(cli)

#endif // _CLI_UI_FLOAT_H_
