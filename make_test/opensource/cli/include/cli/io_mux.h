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
//! @brief IOMux class definition.


#ifndef _CLI_IO_MUX_H_
#define _CLI_IO_MUX_H_

#include "cli/namespace.h"
#include "cli/io_device.h"
#include "cli/shell.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    //! @brief Input / output device multiplexer.
    //!
    //! @warning Please ensure one of the following conditions for each device passed to instances of this class:
    //!     - Either the device is an auto-deleted device,
    //!     - or it will be destroyed after this input / output multiplexer,
    //!     - or it will be dereferenced on termination.
    //! Otherwise you could experience consistency troubles.
    //! The null device and standard devices are not subject to this remark.
    class IOMux : public IODevice
    {
    public:
        //! @brief Main constructor.
        explicit IOMux(
            const bool AutoDelete               //!< Auto-deletion flag.
            );

        //! @brief Destructor.
        virtual ~IOMux(void);

    private:
        //! @brief No default constructor.
        explicit IOMux(void);
        //! @brief No copy constructor.
        IOMux(const IOMux&);
        //! @brief No assignment operator.
        IOMux& operator=(const IOMux&);

    // Outputdevice and IODevice interfaces.
    protected:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool OpenDevice(void);
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool CloseDevice(void);
    public:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void PutString(const char* const STR_Out) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void Beep(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void CleanScreen(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool WouldOutput(const OutputDevice& CLI_Device) const;
        // Inherit doxygen comments from cli::IODevice interface documentation.
        virtual const KEY GetKey(void) const;
        // Inherit doxygen comments from cli::IODevice interface documentation.
        virtual const ResourceString GetLocation(void) const;
        // Inherit doxygen comments from cli::IODevice interface documentation.
        virtual const bool WouldInput(const IODevice& CLI_Device) const;

    public:
        //! @brief Device addition in the list.
        //! @return true if the device has been added, false otherwise.
        const bool AddDevice(
            IODevice* const PCLI_Device          //!< Input / output device.
            );

        //! @brief Current device accessor.
        //! @return Current device.
        const IODevice* const GetCurrentDevice(void) const;

        //! @brief Switch to next device.
        //! @return Next device if success, NULL otherwise.
        const IODevice* const SwitchNextDevice(void);

        //! @brief Reset device list.
        //! @return true for success, false otherwise.
        const bool ResetDeviceList(void);

    protected:
        //! @brief Method called when an input / output device is needed.
        //! @return New input device.
        //! @return NULL means no more input
        //!         unless new input devices have been pushed through AddInput().
        virtual IODevice* const CreateDevice(void);

    private:
        //! @brief Check a current device is ready to be used.
        //! @return Current input device.
        //!
        //! Calls CreateDevice() when the list is empty.
        IODevice* const CheckCurrentDevice(void) const;

        //! @brief Releases first device.
        //! @return true for success, false otherwise.
        const bool ReleaseFirstDevice(void);

    private:
        //! Input / output device list.
        mutable tk::Queue<IODevice*> m_qDevices;
    };

CLI_NS_END(cli)

#endif // _CLI_IO_MUX_H_
