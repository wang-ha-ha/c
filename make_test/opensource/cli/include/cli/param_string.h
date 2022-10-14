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
//! @brief ParamString class definition.

#ifndef _CLI_PARAM_STRING_H_
#define _CLI_PARAM_STRING_H_

#include "cli/namespace.h"
#include "cli/param.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class Help;


    //! @brief String parameter element class.
    class ParamString : public Param
    {
    public:
        //! @brief Constructor.
        explicit ParamString(
            const Help& CLI_Help    //!< Corresponding help.
            );

    protected:
        //! @brief Sub-classes constructor.
        explicit ParamString(
            const char* const STR_Keyword,  //!< Keyword.
            const Help& CLI_Help            //!< Corresponding help.
            );

    public:
        //! @brief Destructor.
        virtual ~ParamString(void);

    private:
        //! @brief No default constructor.
        explicit ParamString(void);
        //! @brief No copy constructor.
        ParamString(const ParamString&);
        //! @brief No assignment operator.
        ParamString& operator=(const ParamString&);

    public:
        //! @brief Implicit cast operator.
        operator const char* const(void) const;

    public:
        // Inherit doxygen comments from cli::Param documentation.
        virtual const bool SetstrValue(const char* const STR_Value) const;

        // Inherit doxygen comments from cli::Param documentation.
        virtual const Param& CopyValue(const Param& CLI_Param) const;

        // Inherit doxygen comments from cli::Param documentation.
        virtual const Param* const Clone(void) const;

    private:
        //! Controled value.
        mutable tk::String m_strValue;
    };

CLI_NS_END(cli)

#endif // _CLI_PARAM_STRING_H_
