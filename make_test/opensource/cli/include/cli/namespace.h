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
//! @brief Namespace management.

#ifndef _CLI_NAMESPACE_H_
#define _CLI_NAMESPACE_H_


//! @brief Begin a namespace definition.
#ifndef CLI_NO_NAMESPACE
    #define CLI_NS_BEGIN(__ns)  namespace __ns {
#else
    #define CLI_NS_BEGIN(__ns)
#endif

//! @brief End a namespace definition.
#ifndef CLI_NO_NAMESPACE
    #define CLI_NS_END(__ns)    }
#else
    #define CLI_NS_END(__ns)
#endif

//! @brief Using namespace directive.
#ifndef CLI_NO_NAMESPACE
    #define CLI_NS_USE(__ns)    using namespace __ns;
#else
    #define CLI_NS_USE(__ns)
#endif

#ifdef CLI_NO_NAMESPACE
    //! @brief Bypass cli namespace symbol.
    #define cli
    //! @brief Bypass cli::tk namespace symbol.
    #define tk
#endif


#endif // _CLI_NAMESPACE_H_

