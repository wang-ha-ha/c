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
//! @brief Debug facilities.

#ifndef _CLI_DEBUG_H_
#define _CLI_DEBUG_H_

#include "cli/namespace.h"
#include "cli/object.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    //! @brief Call information object.
    class CallInfo : public Object
    {
    private:
        //! No default constructor.
        explicit CallInfo(void);

    public:
        //! @warning Copy constructor is declared public for compilation reasons,
        //!          but not implemented.
        CallInfo(const CallInfo&);

        //! @brief Regular constructor.
        explicit CallInfo(
            const char* const STR_File,     //!< File of call.
            const unsigned int I_Line,      //!< Position in file.
            const char* const STR_Function  //!< Function of call.
            );

        //! @brief Destructor.
        ~CallInfo(void);

    private:
        //! @brief No assignment operator.
        CallInfo& operator=(const CallInfo&);

    public:
        //! @brief File accessor.
        //! @return The source file name of the call info.
        const char* const GetFile(void) const;

        //! @brief Line accessor
        //! @return The source line of the call info.
        const unsigned int GetLine(void) const;

        //! @brief Function accessor.
        //! @return The source function of the call info.
        const char* const GetFunction(void) const;

    private:
        const tk::String m_strFile;         //!< File of call.
        const unsigned int m_iLine;         //!< Position in file.
        const tk::String m_strFunction;     //!< Function of call.
    };

    //! @brief Call information filling.
    //!
    //! Use this constant when call information is needed.
    #define __CALL_INFO__ cli::CallInfo(__FILE__, __LINE__, __func__)
        // This hack just to enable compilation with Visual C++
        #if ((defined _MSC_VER) && (! defined __func__))
        #define __func__ ""
        #endif

CLI_NS_END(cli)

#endif // _CLI_DEBUG_H_

