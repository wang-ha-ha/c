#ifndef __CLI_AGENT_API_HPP__
#define __CLI_AGENT_API_HPP__
#include "cli/pch.h"
#include "cli/common.h"

void GenOOB(const cli::OutputDevice& CLI_Out);
void SetKey(const cli::OutputDevice& CLI_Out,const char* key);
void GetKey(const cli::OutputDevice& CLI_Out);
void SetDID(const cli::OutputDevice& CLI_Out,const char* did);
void GetDID(const cli::OutputDevice& CLI_Out);
void SetMAC(const cli::OutputDevice& CLI_Out,const char* mac);
void GetMAC(const cli::OutputDevice& CLI_Out);
void SetSN(const cli::OutputDevice& CLI_Out,const char* sn);
void GetSN(const cli::OutputDevice& CLI_Out);
void GetSYSVerion(const cli::OutputDevice& CLI_Out);
void FactoryReset(const cli::OutputDevice& CLI_Out);
void CamStart(const cli::OutputDevice& CLI_Out);
void CamStop(const cli::OutputDevice& CLI_Out);
void GetQRCode(const cli::OutputDevice& CLI_Out);
void MICSart(const cli::OutputDevice& CLI_Out);
void MICFileUpload(const cli::OutputDevice& CLI_Out);
void MICStop(const cli::OutputDevice& CLI_Out);
void SPKStart(const cli::OutputDevice& CLI_Out);
void SPKStop(const cli::OutputDevice& CLI_Out);
void LightSensorStart(const cli::OutputDevice& CLI_Out);
void LightSensorStop(const cli::OutputDevice& CLI_Out);
void GravitySensorStart(const cli::OutputDevice& CLI_Out);
void WledStart(const cli::OutputDevice& CLI_Out);
void WledStop(const cli::OutputDevice& CLI_Out);
void IRledStart(const cli::OutputDevice& CLI_Out);
void IRledStop(const cli::OutputDevice& CLI_Out);
void RGBLEDSet(const cli::OutputDevice& CLI_Out,const char* colour);
void OBJPirSart(const cli::OutputDevice& CLI_Out);
void OBJPirStop(const cli::OutputDevice& CLI_Out);
void GetBattery(const cli::OutputDevice& CLI_Out);
void ButtonClick(const cli::OutputDevice& CLI_Out);
void SaveInfo(const cli::OutputDevice& CLI_Out,const char* mkey,const char* mvalue);
void GetInfo(const cli::OutputDevice& CLI_Out,const char* mkey);
void MCUUartTest(const cli::OutputDevice& CLI_Out, const char* pcData);
void MCUFactoryTest(const cli::OutputDevice& CLI_Out, int iCmdCode);
void EnterSleepState(const cli::OutputDevice& CLI_Out);
void LEDConfig(const cli::OutputDevice& CLI_Out, int iCmdCode, int iBright, int iRGB);
void MCUPhotoresistancecConfig(const cli::OutputDevice& CLI_Out, int iCmdCode);
void MCUBatteryConfig(const cli::OutputDevice& CLI_Out, int iCmdCode);
void MCUADCConfig(const cli::OutputDevice& CLI_Out, int iCmdCode);
void MCUInfraredConfig(const cli::OutputDevice& CLI_Out, int iCmdCode, int iPWM, int iFrequency);
void MCUTest(const cli::OutputDevice& CLI_Out, int iCmdCode);
void MCUUpgrade(const cli::OutputDevice& CLI_Out);
void MICSpkTest(const cli::OutputDevice& CLI_Out);
void MICSpkAec(const cli::OutputDevice& CLI_Out);
void WIFICMD(const cli::OutputDevice& CLI_Out,const char* chAT);
void zhangsan(const cli::OutputDevice& CLI_Out);
#endif /* __CLI_AGENT_API_HPP__ */
