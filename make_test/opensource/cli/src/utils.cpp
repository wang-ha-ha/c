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


#include "cli/pch.h"

#include <string.h> // memset

#include "utils.h"

CLI_NS_USE(cli)


void cli::CheckSnprintfResult(char* const STR_Buffer, const size_t UI_BufferLength, const int I_SnprintfResult)
{
    // snprintf reference:
    // "The functions snprintf() and vsnprintf() do not write more than size bytes (including the terminating null byte ('\0')).
    //  If the output was truncated due to this limit then the return value is the number of characters (excluding the terminating null byte) which would have been written to the final string if enough space had been available.
    //  Thus, a return value of size or more means that the output was truncated."
    if (I_SnprintfResult < 0)
    {
        // Reset the buffer.
        memset(STR_Buffer, '\0', UI_BufferLength);
    }
    else
    {
        if ((size_t) I_SnprintfResult >= UI_BufferLength)
        {
            // End the buffer with a "...\0" pattern.
            STR_Buffer[UI_BufferLength - 1] = '\0';
            for (   int i = (int) UI_BufferLength - 2;
                    (i >= 0) && (i >= (int) UI_BufferLength - 4);
                    i --)
            {
                STR_Buffer[i] = '.';
            }
        }
    }
}
