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

#include <stdio.h>
#include <math.h>
// See https://stackoverflow.com/questions/8132399/how-to-printf-uint64-t-fails-with-spurious-trailing-in-format#8132440
// "The ISO C99 standard specifies that these macros must only be defined if explicitly requested."
#define __STDC_FORMAT_MACROS
#include <inttypes.h> // PRId64, PRIu64

#include "cli/io_device.h"
#include "cli/cli.h"
#include "cli/string_device.h"
#include "cli/traces.h"
#include "cli/assert.h"
#include "encoding.h"
#include "constraints.h"
#include "utils.h"

CLI_NS_USE(cli)


#ifndef CLI_NO_NAMESPACE
const IOEndl cli::endl;
#else
const IOEndl endl;
#endif

//! @brief Input/Output device instance creation/deletion trace class singleton redirection.
#define TRACE_IO_DEVICE_INSTANCES GetIODeviceInstancesTraceClass()
//! @brief Input/Output device instance creation/deletion trace class singleton.
static const TraceClass& GetIODeviceInstancesTraceClass(void)
{
    static const TraceClass cli_IODeviceInstancesTraceClass("CLI_IO_DEVICE_INSTANCES", Help()
        .AddHelp(Help::LANG_EN, "IO device instance management")
        .AddHelp(Help::LANG_FR, "Gestion des intances de périphériques d'entrée/sortie"));
    return cli_IODeviceInstancesTraceClass;
}
//! @brief Input/Output device opening/closure trace class singleton redirection.
#define TRACE_IO_DEVICE_OPENING GetIODeviceOpeningTraceClass()
//! @brief Input/Output device opening/closure trace class singleton.
static const TraceClass& GetIODeviceOpeningTraceClass(void)
{
    static const TraceClass cli_IODeviceOpeningTraceClass("CLI_IO_DEVICE_OPENING", Help()
        .AddHelp(Help::LANG_EN, "IO device opening management")
        .AddHelp(Help::LANG_FR, "Gestion de l'ouverture des périphériques d'entrée/sortie"));
    return cli_IODeviceOpeningTraceClass;
}


OutputDevice::OutputDevice(
        const char* const STR_DbgName,
        const bool B_AutoDelete)
  : m_strDebugName(MAX_DEVICE_NAME_LENGTH, STR_DbgName),
    m_iInstanceLock(B_AutoDelete ? 0 : 1), m_iOpenLock(0),
    m_cliStringEncoder(* new StringEncoder()),
    m_cliLastError()
{
    // Please, no traces in constructor for consistency reasons.
}

OutputDevice::~OutputDevice(void)
{
    delete & m_cliStringEncoder;
}

const tk::String OutputDevice::GetDebugName(void) const
{
    StringDevice cli_DebugName(MAX_DEVICE_NAME_LENGTH, false);
    cli_DebugName << m_strDebugName << "/" << (const void*) this;
    return cli_DebugName.GetString();
}

const int OutputDevice::UseInstance(const CallInfo& CLI_CallInfo)
{
    GetTraces().SafeTrace(TRACE_IO_DEVICE_INSTANCES, *this)
        << "One more user for instance " << GetDebugName() << ", "
        << "user count: " << m_iInstanceLock << " -> " << m_iInstanceLock + 1 << ", "
        << "from " << CLI_CallInfo.GetFunction() << " "
        << "at " << CLI_CallInfo.GetFile() << ":" << CLI_CallInfo.GetLine() << endl;
    m_iInstanceLock ++;
    return m_iInstanceLock;
}

const int OutputDevice::FreeInstance(const CallInfo& CLI_CallInfo)
{
    GetTraces().SafeTrace(TRACE_IO_DEVICE_INSTANCES, *this)
        << "One less user for instance " << GetDebugName() << ", "
        << "user count: " << m_iInstanceLock << " -> " << m_iInstanceLock - 1 << ", "
        << "from " << CLI_CallInfo.GetFunction() << " "
        << "at " << CLI_CallInfo.GetFile() << ":" << CLI_CallInfo.GetLine() << endl;
    if (m_iInstanceLock == 1)
    {
        GetTraces().SafeTrace(TRACE_IO_DEVICE_INSTANCES, *this)
            << "Deleting the device " << GetDebugName() << endl;
        delete this;
        return 0;
    }
    else
    {
        m_iInstanceLock --;
        CLI_ASSERT(m_iInstanceLock > 0);
        return m_iInstanceLock;
    }
}

const int OutputDevice::GetInstanceUsers(void) const
{
    return m_iInstanceLock;
}

const bool OutputDevice::OpenUp(const CallInfo& CLI_CallInfo)
{
    GetTraces().SafeTrace(TRACE_IO_DEVICE_OPENING, *this)
        << "One more user for instance " << GetDebugName() << ", "
        << "user count: " << m_iOpenLock << " -> " << m_iOpenLock + 1 << ", "
        << "from " << CLI_CallInfo.GetFunction() << " "
        << "at " << CLI_CallInfo.GetFile() << ":" << CLI_CallInfo.GetLine() << endl;

    m_iOpenLock ++;

    if (m_iOpenLock == 1)
    {
        GetTraces().SafeTrace(TRACE_IO_DEVICE_OPENING, *this)
            << "Opening the device " << GetDebugName() << endl;
        if (! OpenDevice())
        {
            return false;
        }
    }

    return true;
}

const bool OutputDevice::CloseDown(const CallInfo& CLI_CallInfo)
{
    bool b_Res = true;

    if (m_iOpenLock > 0)
    {
        GetTraces().SafeTrace(TRACE_IO_DEVICE_OPENING, *this)
            << "One less user for instance " << GetDebugName() << ", "
            << "user count: " << m_iOpenLock << " -> " << m_iOpenLock - 1 << ", "
            << "from " << CLI_CallInfo.GetFunction() << " "
            << "at " << CLI_CallInfo.GetFile() << ":" << CLI_CallInfo.GetLine() << endl;

        if (m_iOpenLock == 1)
        {
            GetTraces().SafeTrace(TRACE_IO_DEVICE_OPENING, *this)
                << "Closing the device " << GetDebugName() << endl;
            b_Res = CloseDevice();
        }

        m_iOpenLock --;
    }
    else
    {
        GetTraces().SafeTrace(TRACE_IO_DEVICE_OPENING, *this)
            << "No more closing down for instance " << GetDebugName() << ", "
            << "user count = " << m_iOpenLock << ", "
            << "from " << CLI_CallInfo.GetFunction() << " "
            << "at " << CLI_CallInfo.GetFile() << ":" << CLI_CallInfo.GetLine() << endl;
    }

    return b_Res;
}

const int OutputDevice::GetOpenUsers(void) const
{
    return m_iOpenLock;
}

// [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
#ifndef CLI_NO_STL
const OutputDevice& OutputDevice::operator <<(const std::string& STR_Out) const
{
    PutString(STR_Out.c_str());
    return *this;
}
#endif

const OutputDevice& OutputDevice::operator <<(const tk::String& STR_Out) const
{
    PutString(STR_Out);
    return *this;
}

const OutputDevice& OutputDevice::operator <<(const char* const STR_Out) const
{
    if (STR_Out != NULL)
    {
        PutString(STR_Out);
    }
    return *this;
}

const OutputDevice& OutputDevice::operator <<(const char C_Out) const
{
    char arc_String[] = { C_Out, '\0' };
    PutString(arc_String);
    return *this;
}

const OutputDevice& OutputDevice::operator <<(const KEY E_Key) const
{
    PutString(m_cliStringEncoder.Encode(E_Key));
    return *this;
}

const OutputDevice& OutputDevice::operator <<(const uint8_t UI8_Out) const
{
    return this->operator<<((uint32_t) UI8_Out);
}

const OutputDevice& OutputDevice::operator <<(const int16_t I16_Out) const
{
    return this->operator<<((int32_t) I16_Out);
}

const OutputDevice& OutputDevice::operator <<(const uint16_t UI16_Out) const
{
    return this->operator<<((uint32_t) UI16_Out);
}

const OutputDevice& OutputDevice::operator <<(const int32_t I32_Out) const
{
    return this->operator<<((int64_t) I32_Out);
}

const OutputDevice& OutputDevice::operator <<(const uint32_t UI32_Out) const
{
    return this->operator<<((uint64_t) UI32_Out);
}

const OutputDevice& OutputDevice::operator <<(const int64_t I64_Out) const
{
    char str_Out[128];
    const int i_Res = snprintf(str_Out, sizeof(str_Out), "%" PRId64, I64_Out);
    CheckSnprintfResult(str_Out, sizeof(str_Out), i_Res);
    PutString(str_Out);
    return *this;
}

const OutputDevice& OutputDevice::operator <<(const uint64_t UI64_Out) const
{
    char str_Out[128];
    const int i_Res = snprintf(str_Out, sizeof(str_Out), "%" PRIu64, UI64_Out);
    CheckSnprintfResult(str_Out, sizeof(str_Out), i_Res);
    PutString(str_Out);
    return *this;
}

const OutputDevice& OutputDevice::operator <<(const float F_Out) const
{
    return this->operator <<((double) F_Out);
}

const OutputDevice& OutputDevice::operator <<(const double D_Out) const
{
    double d_Out = D_Out;

    // Positive / negative number management
    const char* str_Sign = "";
    if (d_Out < 0.0)
    {
        str_Sign = "-";
        d_Out = - d_Out;
    }

    // Reframe very big or very small numbers so that it can be displayed with an appropriate number of digits.
    int i_Exp = 0;
    if (d_Out > 1e6)
    {
        while (d_Out >= 1000.0)
        {
            d_Out /= 1000.0;
            i_Exp += 3;
        }
    }
    if ((d_Out < 1e-6) && (d_Out != 0.0))
    {
        while (d_Out < 1.0)
        {
            d_Out *= 1000.0;
            i_Exp -= 3;
        }
    }

    // Let's print out 6 digits max.
    // Avoid useless trailing '0' in the decimal (after the coma).
    // double d_Decimals = d_Out;
    // d_tmp -= (int) d_tmp;
    int i_Decimal, i_Out;
    for (   i_Decimal = 6, i_Out = (int) floor(d_Out * 1e6 + 0.5); // floor(D+1/2) should be equivalent to round(D)
            i_Decimal > 1;
            i_Decimal --, i_Out /= 10)
    {
        if ((i_Out % 10) != 0)
        {
            break;
        }
    }

    // Format the float number with the computed characteristics.
    char str_Format[128], str_Out[128];
    if (i_Exp == 0)
    {
        snprintf(str_Format, sizeof(str_Format), "%%s%%.%df", i_Decimal); // No call needed to CheckSnprintfResult() here.
        const int i_Res = snprintf(str_Out, sizeof(str_Out), str_Format, str_Sign, d_Out);
        CheckSnprintfResult(str_Out, sizeof(str_Out), i_Res);
    }
    else
    {
        snprintf(str_Format, sizeof(str_Format), "%%s%%.%dfe%%d", i_Decimal); // No call needed to CheckSnprintfResult() here.
        const int i_Res = snprintf(str_Out, sizeof(str_Out), str_Format, str_Sign, d_Out, i_Exp);
        CheckSnprintfResult(str_Out, sizeof(str_Out), i_Res);
    }

    // Output the computed string.
    PutString(str_Out);

    // Return the instance itself.
    return *this;
}

const OutputDevice& OutputDevice::operator <<(const void* const PV_Out) const
{
    char str_Out[128];
    const int i_Res = snprintf(str_Out, sizeof(str_Out), "%p", PV_Out);
    CheckSnprintfResult(str_Out, sizeof(str_Out), i_Res);
    PutString(str_Out);
    return *this;
}

const OutputDevice& OutputDevice::operator <<(const IOEndl& CLI_IOEndl) const
{
    tk::UnusedParameter(CLI_IOEndl); // [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
    PutString("\n");
    return *this;
}

const ResourceString OutputDevice::GetLastError(void) const
{
    return m_cliLastError;
}

OutputDevice& OutputDevice::GetNullDevice(void)
{
    class NullDevice : public OutputDevice
    {
    public:
        explicit NullDevice(void) : OutputDevice("null", false) {}
        virtual ~NullDevice(void) {}

    protected:
        virtual const bool OpenDevice(void) { return true; }
        virtual const bool CloseDevice(void) { return true; }
    public:
        virtual void PutString(const char* const STR_Out) const { cli::tk::UnusedParameter(STR_Out); } // [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
    };

    static NullDevice cli_Null;
    return cli_Null;
}

OutputDevice& OutputDevice::GetStdOut(void)
{
    class StdOutDevice : public OutputDevice
    {
    public:
        explicit StdOutDevice(void) : OutputDevice("stdout", false) {}
        virtual ~StdOutDevice(void) {}

    protected:
        virtual const bool OpenDevice(void) { return true; }
        virtual const bool CloseDevice(void) { return true; }
    public:
        virtual void PutString(const char* const STR_Out) const {
            fprintf(stdout, "%s", STR_Out);
            fflush(stdout);
        }
    };

    static StdOutDevice cli_StdOut;
    return cli_StdOut;
}

OutputDevice& OutputDevice::GetStdErr(void)
{
    class StdErrDevice : public OutputDevice
    {
    public:
        explicit StdErrDevice(void) : OutputDevice("stderr", false) {}
        virtual ~StdErrDevice(void) {}

    protected:
        virtual const bool OpenDevice(void) { return true; }
        virtual const bool CloseDevice(void) { return true; }
    public:
        virtual void PutString(const char* const STR_Out) const {
            fprintf(stderr, "%s", STR_Out);
            fflush(stderr);
        }
    };

    static StdErrDevice cli_StdErr;
    return cli_StdErr;
}

void OutputDevice::Beep(void) const
{
    PutString("\a");
}

void OutputDevice::CleanScreen(void) const
{
    for (int i=0; i<200; i++)
    {
        PutString("\n");
    }
}

const OutputDevice::ScreenInfo OutputDevice::GetScreenInfo(void) const
{
    return ScreenInfo(
        ScreenInfo::UNKNOWN, ScreenInfo::UNKNOWN, // Width and height
        false,  // True Cls
        false   // Line wrapping
    );
}

const bool OutputDevice::WouldOutput(const OutputDevice& CLI_Device) const
{
    return (& CLI_Device == this);
}


IODevice::IODevice(
        const char* const STR_DbgName,
        const bool B_AutoDelete)
  : OutputDevice(STR_DbgName, B_AutoDelete),
    m_cliStringDecoder(* new StringDecoder())
{
}

IODevice::~IODevice(void)
{
    delete & m_cliStringDecoder;
}

IODevice& IODevice::GetNullDevice(void)
{
    class NullDevice : public IODevice
    {
    public:
        explicit NullDevice(void) : IODevice("null", false) {}
        virtual ~NullDevice(void) {}

    protected:
        virtual const bool OpenDevice(void) { return true; }
        virtual const bool CloseDevice(void) { return true; }
    public:
        virtual void PutString(const char* const STR_Out) const { cli::tk::UnusedParameter(STR_Out); } // [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
        virtual const KEY GetKey(void) const { return NULL_KEY; }
    };

    static NullDevice cli_Null;
    return cli_Null;
}

IODevice& IODevice::GetStdIn(void)
{
    class StdInDevice : public IODevice
    {
    public:
        explicit StdInDevice(void) : IODevice("stdin", false) {}
        virtual ~StdInDevice(void) {}

    protected:
        virtual const bool OpenDevice(void) {
            OutputDevice::GetStdOut().UseInstance(__CALL_INFO__);
            return OutputDevice::GetStdOut().OpenUp(__CALL_INFO__);
        }
        virtual const bool CloseDevice(void) {
            bool b_Res = OutputDevice::GetStdOut().CloseDown(__CALL_INFO__);
            OutputDevice::GetStdOut().FreeInstance(__CALL_INFO__);
            return b_Res;
        }
    public:
        virtual void PutString(const char* const STR_Out) const {
            OutputDevice::GetStdOut().PutString(STR_Out);
        }
        virtual void Beep(void) const {
            OutputDevice::GetStdOut().Beep();
        }
        virtual const KEY GetKey(void) const {
            const char c_Char = (char) getchar();
            const KEY e_Key = Char2Key(c_Char);
            if (e_Key != FEED_MORE) {
                return e_Key;
            } else {
                // Recursive call
                return GetKey();
            }
        }
    };

    static StdInDevice cli_StdIn;
    return cli_StdIn;
}

const KEY IODevice::Char2Key(const int I_Char) const
{
    return m_cliStringDecoder.Decode(I_Char);
}

const ResourceString IODevice::GetLocation(void) const
{
    return ResourceString();
}

const bool IODevice::WouldInput(const IODevice& CLI_Device) const
{
    return (& CLI_Device == this);
}
