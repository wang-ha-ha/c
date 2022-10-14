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
//! @brief StringDecoder class defintion.

#ifndef _CLI_ENCODING_H_
#define _CLI_ENCODING_H_

#include "cli/namespace.h"
#include "cli/object.h"
#include "cli/tk.h"
#include "cli/io_device.h"


CLI_NS_BEGIN(cli)


    //! @brief Key to chararcters encoder.
    //!
    //! Outputs utf-8 encoded strings.
    class StringEncoder : public Object
    {
    public:
        //! @brief Default constructor.
        explicit StringEncoder(void);

    private:
        //! @brief No copy constructor.
        explicit StringEncoder(const StringEncoder&);

    public:
        //! @brief Destructor.
        virtual ~StringEncoder(void);

    public:
        //! @brief Key to string encoding.
        //! @return Encoded string.
        const char* const Encode(
            const KEY E_Key                 //!< Key to encode.
            ) const;

    private:
        //! Buffer to receive temporary encoded string.
        mutable tk::String m_tkString;
    };

    //! @brief Regular characters to key decoder.
    //!
    //! Manages special character encoding among others.
    //! utf-8 has the priority, then iso-8859-1 is managed when not conflicting.
    class StringDecoder : public Object
    {
    public:
        //! @brief Default constructor.
        explicit StringDecoder(void);

        //! @brief String based constructor.
        explicit StringDecoder(
            const char* const STR_String    //!< Encoded input string.
            );

    private:
        //! @brief No copy constructor.
        explicit StringDecoder(const StringDecoder&);

    public:
        //! @brief Destructor.
        virtual ~StringDecoder(void);

    public:
        //! @brief Push characters one by one and get the corresponding keys.
        //! @return Corresponding if it can be decoded right away. FEED_MORE if it is part of an ecncoding sequence.
        //!
        //! Basic method used when the instance has been initialized through the default constructor.
        const KEY Decode(
            const int I_Char                //!< Input character (given as an int, the same as fread() behaves)
            );

        //! @brief Retrieve characters from the input string.
        //! @return NULL_KEY when the string has been fully read.
        //!
        //! Based method used when the instance has been initialized through the string based constructor.
        const KEY GetKey(void);

    private:
        //! Encoded input string.
        tk::String m_tkString;
        //! Current sequence memory.
        int m_iCurrentSequence;
    };

CLI_NS_END(cli)

#endif // _CLI_ENCODING_H_
