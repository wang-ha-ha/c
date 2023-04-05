#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>

#include "cli_agent_api.hpp"
#include "cli_agent_mcu.hpp"
#include "cli_agent_common.hpp"
#include "log_mx.h"
#include "fw_env_para.h"

#define _OK "<OK>\n"
#define _ERR "<ERROR>\n"
#define _END "\n"

void QuitTest(const cli::OutputDevice& CLI_Out)
{

}

void WIFICMD(const cli::OutputDevice& CLI_Out,const char* chAT)
{
    std::string  strAT= "fw_cmd \"" + std::string(chAT) + "\"";
    SendToAtbmd(strAT);
    CLI_Out << "<OK>"  ;
}

void GetSYSVerion(const cli::OutputDevice& CLI_Out)
{
    char *version = NULL;
    int len = 0;
    int ret = ReadFile("/etc/version",&version,&len);
    if(ret == 0)
    {
        CLI_Out << _ERR << _END;
    }
    else
    {
        CLI_Out << _OK << version << _END;
    }
    //CLI_Out << m_version.c_str() << cli::endl;
  
    if(version != NULL)
    {
        delete version;
    }
}

void GenOOB(const cli::OutputDevice& CLI_Out)
{
    char oob[258];
    
    memset(oob, 0, sizeof(oob));
    int ret = GetOOB(oob);
    maix::setFWParaConfig("oob",oob,1);
    CLI_Out << _OK ;

}

void SetSN(const cli::OutputDevice& CLI_Out,const char* sn)
{
    // todo 
    // 从env里面去读取和设置,校验一下sn的长度和规则   
    maix::setFWParaConfig("sn",sn,1);
    SetTaskStatus(1);
    CLI_Out << _OK << "SetSN";

}

void GetSN(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "GetSN:" << maix::getFWParaConfig("sn") << cli::endl;
}

void SetKey(const cli::OutputDevice& CLI_Out,const char* key)
{
    maix::setFWParaConfig("key",key,1);
    CLI_Out << "<OK>" << "SetKey: " << cli::endl;
}

void GetKey(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << "<OK>" << "GetKey: " << maix::getFWParaConfig("key") << cli::endl;
}


void SetDID(const cli::OutputDevice& CLI_Out,const char* did)
{
    maix::setFWParaConfig("did",did,1);
    CLI_Out << "<OK>" << "SetDID: " << cli::endl;
}

void GetDID(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << "<OK>" << "GetDID: " << maix::getFWParaConfig("did") << cli::endl;
}

void SetMAC(const cli::OutputDevice& CLI_Out,const char* mac)
{
    // 获取到mac地址之后需要设置到mcu里面 并且写到env里面
    //     ./atbm_iot_cli fw_cmd "AT+WIFI_AP_MAC ADDR 00:11:22:33:44:55"
    //     ./atbm_iot_cli fw_cmd "AT+WIFI_STA_MAC ADDR 00:11:22:33:44:55" 更新了MAC地址之后需要重新上电网卡的mac才会更新
    std::string strMac = "fw_cmd \"AT+WIFI_AP_MAC ADDR " + std::string(mac) + "\"";
    SendToAtbmd(strMac);
    CLI_Out << "<OK>" << "SetMAC" << cli::endl;
}


void GetStrMAC(char* mac)
{
    struct ifreq ifreq;
    int sock;
    if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
    }
    strcpy(ifreq.ifr_name,"wlan0");
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0)
    {
        perror("ioctl");
    }
    sprintf(mac,"%02x%02x%02x%02x%02x%02x",
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
}

void GetMAC(const cli::OutputDevice& CLI_Out)
{
    char mac[18];
    memset(mac, 0, sizeof(mac));
    GetStrMAC(mac);
    CLI_Out << "<OK>" << "GetMAC" << cli::endl << mac <<cli::endl;
}

void FactoryReset(const cli::OutputDevice& CLI_Out)
{
    // 设置 tag_env_info --set HW 70mai_factory_mode 0 设置为0代表为用户模式，退出工厂模式设置为0
    CLI_Out << _ERR << "FactoryReset" << cli::endl;
}

void CamStart(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "CamStart" << cli::endl;
}

void CamStop(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "CamStop" << cli::endl;
}

void GetQRCode(const cli::OutputDevice& CLI_Out)
{
    char QRCode[258];
    char mac[18];
    memset(QRCode, 0, sizeof(QRCode));
    memset(mac, 0, sizeof(mac));
    GetStrMAC(mac);
    sprintf(QRCode,"https://home.mi.com/do/home.html?f=xz&p=76724&d=%s&m=%s&O=%s",maix::getFWParaConfig("did"),maix::getFWParaConfig("oob"),mac);
    
    // 串口通信需要调整，这还是和616通信的那一套，要调整为和高拓通信
        // maix::CFactorySerialCommu* csc = maix::CFactorySerialCommu::GetInstance();
        // std::string qr_code;
        // csc->getQRCode(qr_code);
        // maix::logPrint(maix::MX_LOG_ERROR, "qr_code = %s",qr_code.c_str());
        // CLI_Out << qr_code.c_str()  << cli::endl;

    CLI_Out << "<OK>" << "GetQRCode" << cli::endl << QRCode << cli::endl;
}

void MICSart(const cli::OutputDevice& CLI_Out)
{
    int ret = AudioRecord("/tmp/audio.wav",75);

    if(ret != 0)
    {
        CLI_Out << _ERR << cli::endl;
    }
    else
    {
        CLI_Out << _OK << cli::endl;
    }
}

void MICSpkTest(const cli::OutputDevice& CLI_Out)
{

    int ret = AudioRecord("/tmp/audio.wav",75);
    if(ret != 0)
    {
        CLI_Out << _ERR << cli::endl;
    }
    else
    {
        CLI_Out << _OK << cli::endl;
    }

    ret = AudioAplay("/tmp/audio.wav");
    if(ret != 0)
    {
        CLI_Out << _ERR << cli::endl;
    }
    else
    {
        CLI_Out << _OK << cli::endl;
    }

}

void MICFileUpload(const cli::OutputDevice& CLI_Out)
{
    // 文件是字节流，不是字符串，不能在用CLI_Out直接去打印了，需要找到直接输出字节流的函数
    int fd = open("/tmp/audio.wav",O_RDONLY);
    if(fd <= 0)
    {
        CLI_Out << _ERR;
    }
    else
    {
        CLI_Out << _OK;
    }

    int size = lseek(fd,0,SEEK_END);

    CLI_Out.PutBuffer((char *) &size,4);
    CLI_Out << cli::endl;

    lseek(fd,0,SEEK_SET);

    char buf[512];
    int ret = 0;

    do
    {
        ret = read(fd,buf,sizeof(buf));
        CLI_Out.PutBuffer(buf,sizeof(buf));
    }while(ret > 0);
    
    CLI_Out << cli::endl;
}

void MICStop(const cli::OutputDevice& CLI_Out)
{    
    // 删掉
    CLI_Out << _OK << "MICStop" << cli::endl;
}

void RGBLEDSet(const cli::OutputDevice& CLI_Out,const char* colour)
{
    // set 616 led color
    CLI_Out << _OK << "RGBLEDSet" << cli::endl;
}

void SPKStart(const cli::OutputDevice& CLI_Out)
{
    int ret = AudioAplay("/system/snd/speaker_to_mic_test.wav");

    if(ret != 0)
    {
        CLI_Out << _ERR << cli::endl;
    }
    else
    {
        CLI_Out << _OK << cli::endl;
    }
}

void MICSpkAec(const cli::OutputDevice& CLI_Out)
{
    int result = 0;

	std::thread* SPKStart;
   // SPKStart = new std::thread([&] { result = SPKStart("/system/snd/speaker_to_mic_test.wav"); });




    //std::thread SPKStart(AudioAplay,"/system/snd/speaker_to_mic_test.wav");

    int ret = AudioRecord("/tmp/audio.wav",75);
    if(ret != 0)
    {
        CLI_Out << _ERR << cli::endl;
    }
    else
    {
        CLI_Out << _OK << cli::endl;
    }


    SPKStart->join();
}


void SPKStop(const cli::OutputDevice& CLI_Out)
{
    // 删掉
    CLI_Out << _OK << "SPKStop" << cli::endl;
}

void LightSensorStart(const cli::OutputDevice& CLI_Out)
{

    CLI_Out << _OK << "LightSensorStart" << cli::endl;

    maix::logPrint(maix::MX_LOG_ERROR, "LightSensorStart tid = %u",pthread_self());

    std::string cmd1 = "insmod /system/lib/modules/ltr-311als.ko";
    system(cmd1.c_str());
    sleep(1);

    std::string cmd2 = "echo 1 >  /sys/devices/platform/apb/10051000.i2c/i2c-1/1-0022/ltr311als/enable";
    system(cmd2.c_str());
    sleep(1);


    std::string cmd3 = "/sys/devices/platform/apb/10051000.i2c/i2c-1/1-0022/ltr311als/value";
    int fd = -1;
    int ret = -1;
    char buf[20] = {0};
    FILE* file = fopen(cmd3.c_str(),"r");
    if(file == NULL)
    {
        maix::logPrint(maix::MX_LOG_ERROR, "open file failed!");
    }

    //int count = 20; 
    //while(count-- > 0)
    
    sleep(1);
    fgets(buf,20,file);
    maix::logPrint(maix::MX_LOG_ERROR, "buf  = %s",buf);
    CLI_Out << buf << cli::endl;
    
    fclose(file);
    file = NULL;

    std::string cmd4 = "echo 0 >  /sys/devices/platform/apb/10051000.i2c/i2c-1/1-0022/ltr311als/enable";
    system(cmd4.c_str());
}

void LightSensorStop(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "LightSensorStop" << cli::endl;

    maix::logPrint(maix::MX_LOG_ERROR, "LightSensorStop tid = %u",pthread_self());
}

void GravitySensorStart(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "GravitySensorStart" << cli::endl;
}

void WledStart(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "WledStart" << cli::endl;
    std::string cmd = "/system/bin/wled on";
    system(cmd.c_str());
}

void WledStop(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "WledStop" << cli::endl;
    std::string cmd = "/system/bin/wled off";
    system(cmd.c_str());
}

void IRledStart(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "IRledStart" << cli::endl;
    std::string cmd = "/system/bin/irled on";
    system(cmd.c_str());
}

void IRledStop(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _OK << "IRledStop" << cli::endl;
    std::string cmd = "/system/bin/irled off";
    system(cmd.c_str());
}

void OBJPirSart(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _ERR << cli::endl;
}

void OBJPirStop(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _ERR << cli::endl;
}


void GetBattery(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _ERR << cli::endl;
}

void ButtonClick(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << _ERR << cli::endl;
}

void SaveInfo(const cli::OutputDevice& CLI_Out,const char* mkey,const char* mvalue)
{
    CLI_Out << _ERR << cli::endl;
}

void GetInfo(const cli::OutputDevice& CLI_Out,const char* mkey)
{
    // 存储每条测试指令的结果到一个文件里面，以key=value的形式存储，直接将文件返回
    CLI_Out << _ERR << "GetInfo" << cli::endl;
}

void MCUUpgrade(const cli::OutputDevice& CLI_Out)
{
    //升级
    // CFactoryManageModule * cfm = CFactoryManageModule::GetInstance();
    // cfm->ToolsCLI("update_fw OTA_6441_zip.bin");

    //设置
    std::string cmd = "fw_setenv factory_mode 1";
    system(cmd.c_str());

    //重启
    // cfm->ToolsCLI("fw_cmd \"AT+REBOOT\"");
}

void MCUUartTest(const cli::OutputDevice& CLI_Out, const char* pcData)
{
    CLI_Out << "get param: " <<  pcData<< cli::endl;

}

void MCUFactoryTest(const cli::OutputDevice& CLI_Out, int iCmdCode)
{
    CLI_Out << "get param: " <<  iCmdCode << cli::endl;
}

void EnterSleepState(const cli::OutputDevice& CLI_Out)
{
    CLI_Out << "start proc sleep state " << cli::endl;
    CLI_Out << "end proc sleep state " << cli::endl;
}

void LEDConfig(const cli::OutputDevice& CLI_Out, int iCmdCode, int iBright, int iRGB)
{
    CLI_Out << "param: " << iCmdCode << iBright << iRGB << cli::endl;
}

void MCUPhotoresistancecConfig(const cli::OutputDevice& CLI_Out, int iCmdCode)
{
    CLI_Out << "param: " << iCmdCode << cli::endl;
}

void MCUBatteryConfig(const cli::OutputDevice& CLI_Out, int iCmdCode)
{
    CLI_Out << "param: " << iCmdCode << cli::endl;
}

void MCUADCConfig(const cli::OutputDevice& CLI_Out, int iCmdCode)
{
    CLI_Out << "param: " << iCmdCode << cli::endl;
}

void MCUInfraredConfig(const cli::OutputDevice& CLI_Out, int iCmdCode, int iPWM, int iFrequency)
{
    CLI_Out << "param: " << iCmdCode << cli::endl;
}

void MCUTest(const cli::OutputDevice& CLI_Out, int iCmdCode)
{
    maix::CCliAgentMcu *CliAgentMcu = maix::CCliAgentMcuFactory::GetInstance(MCU_TYPE);
    maix::CCliAgentMcuIoDevice *CliAgentMcuIo = CliAgentMcu->GetIO();

    char buf[128] = {1,2,3,4,5,6,7,8,9,10};
    int ret = CliAgentMcuIo->SendBuf((unsigned char *)buf,10);

    cli::OutputDevice::GetStdErr() << "send ret:" << ret << cli::endl;

    CLI_Out << _OK << "AAA" << ret << "AAA";

    memset(buf,0,sizeof(buf));
    ret = CliAgentMcuIo->RecvBuf((unsigned char *)buf,sizeof(buf),3000);

    cli::OutputDevice::GetStdErr() << "recv ret:" << ret << cli::endl;

    CLI_Out << "AAA" << ret << "AAA"  << buf << cli::endl;
}