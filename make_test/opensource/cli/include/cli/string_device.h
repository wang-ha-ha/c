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
//! @brief String device definition.

#ifndef _CLI_STRING_DEVICE_H_
#define _CLI_STRING_DEVICE_H_


#include "cli/tk.h"
#include "cli/io_device.h"

CLI_NS_BEGIN(cli)

    //! @brief String device.
    //!
    //! Unless for unit tests purpose, I don't really know yet what this device could be useful for.
    //! However, if one would need to receive the result of commands in strings to analyze them, this device would fit this kind of need.
    //! That's the reason why I'm exposing it in the public visibility like the other classes of the library.
    class StringDevice : public OutputDevice
    {
    public:
        //! @brief Regular constructor.
        explicit StringDevice(
            const unsigned int UI_OutputMaxLen, //!< Output maximum length.
            const bool B_AutoDelete             //!< Auto-deletion flag.
            );

        //! @brief Destructor.
        virtual ~StringDevice(void);

    private:
        //! @brief No default constructor.
        explicit StringDevice(void);
        //! @brief No copy constructor.
        StringDevice(const StringDevice&);
        //! @brief No assignment operator.
        StringDevice& operator=(const StringDevice&);

    public:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool OpenDevice(void);
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool CloseDevice(void);
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void PutString(const char* const STR_Out) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void Beep(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void CleanScreen(void) const;

    public:
        //! @brief String accessor.
        //! @return Output string.
        const tk::String& GetString(void) const;

        //! @brief Reset.
        //! @return The StringDevice instance itself.
        StringDevice& Reset(void);

    private:
        //! String managed by this device.
        mutable tk::String m_tkString;
    };

CLI_NS_END(cli)

#endif // _CLI_STRING_DEVICE_H_
