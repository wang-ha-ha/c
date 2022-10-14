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
//! @brief Execution context manager class definition.

#ifndef _CLI_EXEC_CONTEXT_MANAGER_H_
#define _CLI_EXEC_CONTEXT_MANAGER_H_

#include "cli/namespace.h"
#include "cli/exec_context.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class OutputDevice;
    class IODevice;
    class TraceClass;


    //! @brief Execution context trace class singleton redirection.
    #define TRACE_EXEC_CTX ExecutionContextManager::GetTraceClass()

    //! @brief Execution context manager class.
    class ExecutionContextManager
    {
    public:
        //! @brief Execution context trace class singleton.
        //! @return Execution context trace class instance.
        static const TraceClass& GetTraceClass(void);

    public:
        //! @brief Default constructor.
        explicit ExecutionContextManager(void);

        //! @brief Destructor.
        virtual ~ExecutionContextManager(void);

    private:
        //! @brief No copy constructor.
        ExecutionContextManager(const ExecutionContextManager&);
        //! @brief No assignment operator.
        ExecutionContextManager& operator=(const ExecutionContextManager&);

    public:
        //! @brief Instance registration.
        void UseInstance(
            ExecutionContext& CLI_UserInstance  //!< Instance to register.
            );

        //! @brief Instance deregistration.
        //! @warning Shall auto-delete the manager when no more instances registered.
        void FreeInstance(
            ExecutionContext& CLI_UserInstance  //!< Instance to deregister.
            );

    public:
        //! @brief Stop execution of all running contexts.
        void StopAllExecutions(void);

    public:
        //! @brief Assumes input and output devices to be ready for execution.
        //! @return true for success, false otherwise.
        const bool OpenUp(
            ExecutionContext& CLI_Context,  //!< Execution context starting its execution.
            IODevice& CLI_IODevice          //!< Input/output device to run onto.
        );

        //! @brief Release input and output devices after execution.
        //! @return true for success, false otherwise.
        const bool CloseDown(
            ExecutionContext& CLI_Context   //!< Execution context terminating its execution.
            );

        //! @brief Determines whether the execution context manager is currently running.
        //! @return true if the execution context manager is currently running, false otherwise.
        const bool IsRunning(void) const;

        //! @brief Determines whether the given execution context is currently running.
        //! @return true if the given execution context is currently running, false otherwise.
        const bool IsRunning(
            const ExecutionContext& CLI_Context //!< Execution context to look for execution.
            ) const;

        //! @brief Returns the current execution context.
        //! @return Current execution context if any, NULL otherwise.
        ExecutionContext* const GetCurrentContext(void);

    public:
        //! @brief Input stream accessor.
        //! @return Input stream reference.
        const IODevice& GetInput(void) const;

        //! @brief Output stream accessor.
        //! @return Output stream reference.
        const OutputDevice& GetStream(
            const STREAM_TYPE E_StreamType          //!< Output stream identifier.
            ) const;

        //! @brief Output stream positionning.
        //! @return true for success, false otherwise.
        //! @warning Please ensure one of the following conditions regarding the given device:
        //!     - Either the device is an auto-deleted device,
        //!     - or it will be destroyed after this execution context object,
        //!     - or another call to this method with the null device is done on termination.
        //! Otherwise you could experience consistency troubles.
        //! The null device and standard devices are not subject to this remark.
        const bool SetStream(
            const STREAM_TYPE E_StreamType,         //!< Output stream identifier.
            OutputDevice& CLI_Stream                //!< Stream reference.
            );

        //! @brief Enabled/disabled stream accessor.
        //! @return true if the stream is enabled, false otherwise.
        const bool StreamEnabled(
            const STREAM_TYPE E_StreamType          //!< Output stream identifier.
            ) const;

        //! @brief Enable/disable stream.
        //! @return true for success, false otherwise.
        const bool EnableStream(
            const STREAM_TYPE E_StreamType,         //!< Output stream identifier.
            const bool B_Enable                     //!< Enable flag.
            );

    public:
        //! @brief Language setting.
        void SetLang(
            const ResourceString::LANG E_Lang       //!< New value.
            );

        //! @brief Language access.
        //! @return The language currently set.
        const ResourceString::LANG GetLang(void) const;

    public:
        //! @brief Beep configuration setting.
        void SetBeep(
            const bool B_Enable                     //!< New value.
            );

        //! @brief Beep configuration access.
        //! @return The current beep configuration. true if enabled, false otherwise.
        const bool GetBeep(void) const;

        //! @brief Sends a beep signal.
        void Beep(void);

    private:
        //! @brief Ensure input device release.
        void ReleaseInput(void);

    private:
        //! Input device.
        IODevice* m_pcliInput;
        //! Output streams.
        struct {
            OutputDevice* pcliStream;
            bool bEnable;
        } m_artStream[STREAM_TYPES_COUNT];
        //! Current language.
        ResourceString::LANG m_eLang;
        //! Beep enabled.
        bool m_bBeep;
        //! Registered execution contexts.
        tk::Queue<ExecutionContext*> m_tkUserInstances;
        //! Currently running execution contexts.
        tk::Queue<ExecutionContext*> m_tkRunningContexts;
    };

CLI_NS_END(cli)

#endif // _CLI_EXEC_CONTEXT_MANAGER_H_
