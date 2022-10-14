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
//! @brief CLI toolkit definition.

#ifndef _CLI_TK_H_
#define _CLI_TK_H_

#include <stdlib.h>

#include "cli/namespace.h"


CLI_NS_BEGIN(cli)

    //! @namespace cli::tk
    //! @brief CLI classes toolkit.
    CLI_NS_BEGIN(tk)

        //! @brief Dummy function that aims to avoid warnings for unused parameters.
        //!
        //! Does strictly nothing, but avoids warnings for unused parameters.
        template <class T> const void* UnusedParameter(
            const T& T_UnusedParam     //!< Unused parameter to avoid warnings for.
            )
        {
            return (& T_UnusedParam);
        }

    CLI_NS_END(tk)

CLI_NS_END(cli)


#ifndef CLI_NO_STL
    #include "cli/tk_stl.h"
#else
    #include "cli/tk_inner.h"
#endif


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(tk)

        //! @brief Comparison operator.
        //! @return true if strings are equal, false otherwise.
        const bool operator==(
                const char* const STR_String1,  //!< First string to compare.
                const tk::String& STR_String2   //!< Second string to compare.
                );

        //! @brief Comparison operator.
        //! @return true if strings are equal, false otherwise.
        const bool operator==(
                const tk::String& STR_String1,  //!< First string to compare.
                const char* const STR_String2   //!< Second string to compare.
                );

        //! @brief Comparison operator.
        //! @return true if strings are equal, false otherwise.
        const bool operator==(
                const tk::String& STR_String1,  //!< First string to compare.
                const tk::String& STR_String2   //!< Second string to compare.
                );

        //! @brief Difference operator.
        //! @return true if strings differ, false otherwise.
        const bool operator!=(
                const char* const STR_String1,  //!< First string to compare.
                const tk::String& STR_String2   //!< Second string to compare.
                );

        //! @brief Difference operator.
        //! @return true if strings differ, false otherwise.
        const bool operator!=(
                const tk::String& STR_String1,  //!< First string to compare.
                const char* const STR_String2   //!< Second string to compare.
                );

        //! @brief Difference operator.
        //! @return true if strings differ, false otherwise.
        const bool operator!=(
                const tk::String& STR_String1,  //!< First string to compare.
                const tk::String& STR_String2   //!< Second string to compare.
                );

        //! @brief Lower operator.
        //! @return true string1 is "lower than" string2, false otherwise.
        const bool operator<(
                const tk::String& STR_String1,  //!< Supposed lower string.
                const tk::String& STR_String2   //!< Supposed upper string.
                );

        //! @brief Greater operator.
        //! @return true string1 is "greater than" string2, false otherwise.
        const bool operator>(
                const tk::String& STR_String1,  //!< Supposed upper string.
                const tk::String& STR_String2   //!< Supposed lower string.
                );

        //! @brief Lower or equal operator.
        //! @return true string1 is "lower than" or equals string2, false otherwise.
        const bool operator<=(
                const tk::String& STR_String1,  //!< Supposed lower string.
                const tk::String& STR_String2   //!< Supposed upper string.
                );

        //! @brief Greater or equal operator.
        //! @return true string1 is "greater than" or equals string2, false otherwise.
        const bool operator>=(
                const tk::String& STR_String1,  //!< Supposed upper string.
                const tk::String& STR_String2   //!< Supposed lower string.
                );

    CLI_NS_END(tk)

CLI_NS_END(cli)

#endif // _CLI_TK_H_

