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
//! @brief NonBlockingDevice class definition.


#ifndef _CLI_NON_BLOCKING_DEVICE_H_
#define _CLI_NON_BLOCKING_DEVICE_H_

#include "cli/namespace.h"
#include "cli/object.h"
#include "cli/io_device.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class ExecutionContext;
    class ExecutionContextManager;


    //! @brief Non-blocking input device.
    class NonBlockingIODevice : public IODevice
    {
    public:
        //! @brief Main constructor.
        explicit NonBlockingIODevice(
            const char* const STR_DbgName,  //!< Debug name.
            const bool B_AutoDelete         //!< Auto-deletion flag.
            );

        //! @brief Destructor.
        virtual ~NonBlockingIODevice(void);

    private:
        //! @brief No default constructor.
        explicit NonBlockingIODevice(void);
        //! @brief No copy constructor.
        NonBlockingIODevice(const NonBlockingIODevice&);
        //! @brief No assignment operator.
        NonBlockingIODevice& operator=(const NonBlockingIODevice&);

    public:
        // Inherit doxygen comments from cli::IODevice interface documentation.
        virtual const KEY GetKey(void) const;

    private:
        //! @brief Sets an execution context manager reference.
        void SetExecutionContextManager(
            ExecutionContextManager* const PCLI_ExecutionContextManager //!< Execution context manager reference. May be NULL.
            );

    protected:
        //! @brief Returns the current execution context.
        //! @return Current execution context if any, NULL otherwise.
        const ExecutionContext* const GetExecutionContext(void) const;

    public:
        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief Handler to call when a key is received.
        //! @param E_Key Input key.
        virtual void OnKey(const KEY E_Key) const;

    private:
        //! Execution context manager reference.
        ExecutionContextManager* m_pcliExecutionContextManager;

    private:
        friend class ExecutionContextManager; // SetExecutionContextManager()
    };

CLI_NS_END(cli)

#endif // _CLI_NON_BLOCKING_DEVICE_H_
