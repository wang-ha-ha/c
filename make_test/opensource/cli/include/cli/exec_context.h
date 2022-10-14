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
//! @brief Execution context class definition.

#ifndef _CLI_EXEC_CONTEXT_H_
#define _CLI_EXEC_CONTEXT_H_

#include "cli/namespace.h"
#include "cli/io_device.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class ExecutionContextManager;
    class ExecutionResult;


    //! @brief Output stream enumeration.
    //! @return N/A (doxygen warning)
    typedef enum _STREAM_TYPE
    {
        ALL_STREAMS = -1,   //!< All streams.

        WELCOME_STREAM = 0, //!< Welcome stream. Useful for bye too.
        PROMPT_STREAM,      //!< Prompt stream.
        ECHO_STREAM,        //!< Echo stream.
        OUTPUT_STREAM,      //!< Output stream.
        ERROR_STREAM,       //!< Error stream.
        STREAM_TYPES_COUNT  //!< Number of streams.
    } STREAM_TYPE;


    //! @brief Execution context.
    //!
    //! An execution context, in the CLI library, is something that manages:
    //!     - a set of input/output devices,
    //!     - settings: language, beep...
    //!     - input character processing
    //!
    //!
    //! <h1>Input character processing</h1>
    //!
    //! Input character processing is implemented through the Run method.
    //! It may be either blocking or non blocking depending on the type of input/output device given.
    //!
    //! When the input/output device is a blocking device (IODevice based class), the execution is done whithin a blocking call
    //! as illustrated by the following chart:
    //!     @msc
    //!     User,ExecutionContext,IODevice;
    //!     User=>ExecutionContext                  [label="Run(IODevice&)", URL="\ref ExecutionContext::Run()"];
    //!     ExecutionContext=>IODevice              [label="GetKey()", URL="\ref IODevice::GetKey()"];
    //!     ExecutionContext=>IODevice              [label="GetKey()", URL="\ref IODevice::GetKey()"];
    //!     ExecutionContext=>IODevice              [label="GetKey()", URL="\ref IODevice::GetKey()"];
    //!     User<<ExecutionContext                  [label="Run(IODevice&)", URL="\ref ExecutionContext::Run()"];
    //!     @endmsc
    //! When the input/output device is a non blocking device (NonBlockingIODevice based class), the ExecutionContext::Run() is non blocking,
    //! and the execution context waits for keys to be notified by the user's integration.
    //! Execution result is notified through the ExecutionResult callback interface:
    //!     @msc
    //!     User,NonBlockingIODevice,ExecutionContext,ExecutionResult;
    //!     User=>ExecutionResult                   [label="WatchResult(ExecutionContext&)", URL="\ref ExecutionResult::WatchResult()"];
    //!     User=>ExecutionContext                  [label="Run(NonBlockingIODevice&)", URL="\ref ExecutionContext::Run()"];
    //!     User<<ExecutionContext                  [label="Run(NonBlockingIODevice&)", URL="\ref ExecutionContext::Run()"];
    //!     User=>NonBlockingIODevice               [label="OnKey()", URL="\ref NonBlockingIODevice::OnKey()"];
    //!     NonBlockingIODevice=>>ExecutionContext  [label="OnKey()", URL="\ref ExecutionContext::OnKey()"];
    //!     User=>NonBlockingIODevice               [label="OnKey()", URL="\ref NonBlockingIODevice::OnKey()"];
    //!     NonBlockingIODevice=>>ExecutionContext  [label="OnKey()", URL="\ref ExecutionContext::OnKey()"];
    //!     User=>NonBlockingIODevice               [label="OnKey()", URL="\ref NonBlockingIODevice::OnKey()"];
    //!     NonBlockingIODevice=>>ExecutionContext  [label="OnKey()", URL="\ref ExecutionContext::OnKey()"];
    //!     ExecutionResult<<=ExecutionContext      [label="OnResult(ExecutionContext&)", URL="\ref ExecutionResult::OnResult()"];
    //!     @endmsc
    //!
    //!
    //! <h1>Top and child execution contexts</h1>
    //!
    //! An execution context may be either a top or child execution context, depending on the kind of constructor actually called.
    //! A child execution context uses the same context as its parent.
    //!
    //! When execution takes place in a blocking call, the flow charts is straight forward:
    //!     @msc
    //!     User,ExecutionContext1,ExecutionContext2,IODevice;
    //!     User=>ExecutionContext1                 [label="new ExecutionContext()", URL="\ref ExecutionContext::ExecutionContext()"];
    //!     User=>ExecutionContext1                 [label="Run(IODevice&)", URL="\ref ExecutionContext::Run()"];
    //!     ExecutionContext1=>IODevice             [label="GetKey()", URL="\ref IODevice::GetKey()"];
    //!     ExecutionContext1=>IODevice             [label="GetKey()", URL="\ref IODevice::GetKey()"];
    //!     User<<=ExecutionContext1                [label="(callback)"];
    //!     ---                                     [label="Beginning of ExecutionContext2's execution"];
    //!     User=>ExecutionContext2                 [label="new ExecutionContext(ExecutionContext1&)", URL="\ref ExecutionContext::ExecutionContext()"];
    //!     User=>ExecutionContext2                 [label="Run()", URL="\ref ExecutionContext::Run()"];
    //!     ExecutionContext2=>IODevice             [label="GetKey()", URL="\ref IODevice::GetKey()"];
    //!     ExecutionContext2=>IODevice             [label="GetKey()", URL="\ref IODevice::GetKey()"];
    //!     User<<ExecutionContext2                 [label="Run()", URL="\ref ExecutionContext::Run()"];
    //!     ---                                     [label="End of ExecutionContext2's execution"];
    //!     ExecutionContext1=>IODevice             [label="GetKey()", URL="\ref IODevice::GetKey()"];
    //!     User<<ExecutionContext1                 [label="Run(IODevice&)", URL="\ref ExecutionContext::Run()"];
    //!     @endmsc
    //! When it takes place in a non blocking call, the input/output device automatically forwards input keys to the current active context:
    //!     @msc
    //!     User,NonBlockingIODevice,ExecutionContext1,ExecutionContext2;
    //!     User=>ExecutionContext1                 [label="new ExecutionContext()", URL="\ref ExecutionContext::ExecutionContext()"];
    //!     User=>ExecutionContext1                 [label="Run(NonBlockingIODevice&)", URL="\ref ExecutionContext::Run()"];
    //!     User<<ExecutionContext1                 [label="Run(NonBlockingIODevice&)", URL="\ref ExecutionContext::Run()"];
    //!     User=>NonBlockingIODevice               [label="OnKey()", URL="\ref NonBlockingIODevice::OnKey()"];
    //!     NonBlockingIODevice=>>ExecutionContext1 [label="OnKey()", URL="\ref ExecutionContext::OnKey()"];
    //!     User=>NonBlockingIODevice               [label="OnKey()", URL="\ref NonBlockingIODevice::OnKey()"];
    //!     NonBlockingIODevice=>>ExecutionContext1 [label="OnKey()", URL="\ref ExecutionContext::OnKey()"];
    //!     User<<=ExecutionContext1                [label="(callback)"];
    //!     ---                                     [label="Beginning of ExecutionContext2's execution"];
    //!     User=>ExecutionContext2                 [label="new ExecutionContext(ExecutionContext1&)", URL="\ref ExecutionContext::ExecutionContext()"];
    //!     User=>ExecutionContext2                 [label="Run()", URL="\ref ExecutionContext::Run()"];
    //!     User<<ExecutionContext2                 [label="Run()", URL="\ref ExecutionContext::Run()"];
    //!     User=>NonBlockingIODevice               [label="OnKey()", URL="\ref NonBlockingIODevice::OnKey()"];
    //!     NonBlockingIODevice=>>ExecutionContext2 [label="OnKey()", URL="\ref ExecutionContext::OnKey()"];
    //!     User=>NonBlockingIODevice               [label="OnKey()", URL="\ref NonBlockingIODevice::OnKey()"];
    //!     NonBlockingIODevice=>>ExecutionContext2 [label="OnKey()", URL="\ref ExecutionContext::OnKey()"];
    //!     ---                                     [label="End of ExecutionContext2's execution"];
    //!     User=>NonBlockingIODevice               [label="OnKey()", URL="\ref NonBlockingIODevice::OnKey()"];
    //!     NonBlockingIODevice=>>ExecutionContext1 [label="OnKey()", URL="\ref ExecutionContext::OnKey()"];
    //!     @endmsc
    class ExecutionContext : public Object
    {
    protected:
        //! @brief Top context constructor.
        explicit ExecutionContext(void);

        //! @brief Child context constructor.
        //!
        //! Makes this execution context run in the same context as the one given for parent.
        explicit ExecutionContext(
            ExecutionContext& CLI_ParentContext //!< Parent execution context.
            );

    public:
        //! @brief Destructor.
        virtual ~ExecutionContext(void);

    private:
        //! @brief No assignment operator.
        ExecutionContext& operator=(const ExecutionContext&);

    public:
        //! @brief Execution context manager accessor.
        //! @return Execution context manager reference.
        ExecutionContextManager& GetContextManager(void);

    public:
        //! @brief Input stream accessor.
        //! @return The input stream attached to the context for its execution.
        const IODevice& GetInput(void) const;

        //! @brief Output stream accessor.
        //! @return The required stream. null if an error occurred.
        const OutputDevice& GetStream(
            const STREAM_TYPE E_StreamType          //!< Output stream identifier.
            ) const;

        //! @brief Output stream positionning.
        //! @return true: success, false: failure.
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
        //! @return true: the stream is enabled, false: the stream is disabled.
        const bool StreamEnabled(
            const STREAM_TYPE E_StreamType          //!< Output stream identifier.
            ) const;

        //! @brief Enable/disable stream.
        //! @return true: success, false: failure.
        const bool EnableStream(
            const STREAM_TYPE E_StreamType,         //!< Output stream identifier.
            const bool B_Enable                     //!< Enable flag.
            );

    public:
        //! @brief Language setting.
        void SetLang(
            const ResourceString::LANG E_Lang   //!< New value.
            );

        //! @brief Language access.
        //! @return The language currently set.
        const ResourceString::LANG GetLang(void) const;

    public:
        //! @brief Beep configuration setting.
        void SetBeep(
            const bool B_Enable         //!< New value.
            );

        //! @brief Beep configuration access.
        //! @return The current beep configuration. true if enabled, false otherwise.
        const bool GetBeep(void) const;

        //! @brief Sends a beep signal.
        void Beep(void);

    public:
        //! @brief Runs the execution context onto the corresponding input/output device.
        //!
        //! Call this runner method, for top execution contexts.
        void Run(
            IODevice& CLI_IODevice                             //!< Input/output device to run onto.
            );

        //! @brief Runs the execution context as a child context of the parent given by the construction.
        //!
        //! Call this runner method, for child execution contexts.
        void Run(void);

        //! @brief Tells whether this execution context is running or not.
        //! @return The running status.
        const bool IsRunning(void) const;

    public:
        //! @brief Terminates this execution context's execution.
        //! @warning Not thread safe. Implement a non blocking device if your CLI execution should interact with external events.
        void StopExecution(void);

        //! @brief Terminates execution for all execution contexts attached to the same top execution context.
        //! @warning Not thread safe. Implement a non blocking device if your CLI execution should interact with external events.
        void StopAllExecutions(void);

    protected:
        //! @brief Beginning of execution handler.
        //! @return true for success, false otherwise.
        virtual const bool OnStartExecution(void) = 0;

        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief Handler called on character input.
        //! @param E_KeyCode Input key.
        virtual void OnKey(const KEY E_KeyCode) = 0;

        //! @brief Execution termination handler.
        //! @return true for success, false otherwise.
        virtual const bool OnStopExecution(void) = 0;

    private:
        //! @brief Beginning of execution.
        //! @return true for success, false otherwise.
        const bool BeginExecution(
            IODevice& CLI_IODevice              //!< Input/output device to run onto.
            );

        //! @brief Main loop executed when the input device is not a non-blocking device.
        void MainBlockingLoop(void);

        //! @brief Key processing.
        void ProcessKey(
            const KEY E_KeyCode                 //!< Input key.
            );

        //! @brief End of execution.
        //! @return true for success, false otherwise.
        const bool FinishExecution(void);

    private:
        //! Execution context manager.
        ExecutionContextManager& m_cliManager;

        //! Execution result interfaces.
        tk::Queue<ExecutionResult*> m_tkResults;

    private:
        // ExecutionContext and NonBlockingIODevice are friends so that they can interact with non-public entry points.
        friend class NonBlockingIODevice;

        // ExecutionContext and ExecutionResult as well.
        friend class ExecutionResult;
    };


    //! @brief Result interface for execution contexts.
    //!
    //! This interface implements callbacks on execution contexts termination.
    class ExecutionResult : public Object
    {
    protected:
        //! @brief Default constructor.
        explicit ExecutionResult(void);

    public:
        //! @brief Pure virtual destructor.
        virtual ~ExecutionResult(void) = 0;

    private:
        //! @brief No Copy constructor.
        ExecutionResult(const ExecutionResult&);
        //! @brief No assignment operator.
        ExecutionResult& operator=(const ExecutionResult&);

    public:
        //! @brief Start watching the execution context for a result.
        //! @note Watching of the context ends as soon as the end of execution is notified.
        void WatchResult(
            ExecutionContext& CLI_Context   //!< Execution context to be watched.
            );

    protected:
        //! @brief User interface result handler.
        virtual void OnResult(
            const ExecutionContext& CLI_Context     //!< Execution context which execution is done.
            ) = 0;

    private:
        friend class ExecutionContext;
    };

CLI_NS_END(cli)

#endif // _CLI_EXEC_CONTEXT_H_

