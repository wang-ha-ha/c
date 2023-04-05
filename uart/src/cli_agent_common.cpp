#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "cli_agent_common.hpp"
#include "log_mx.h"

CZeratulAudioInterface *AiInterface = NULL;
CZeratulSpeakerInterface *AoInterface = NULL;

cli::Cli* const GetCli(void)
{
    cli::Cli::List cli_CLIs(1);
    if ((cli::Cli::FindFromName(cli_CLIs, ".*") <= 0) || (cli_CLIs.IsEmpty()))
    {
        cli::OutputDevice::GetStdErr() << "No CLI found" << cli::endl;
    }
    else
    {
        if (const cli::Cli* const pcli_Cli = cli_CLIs.GetHead())
        {
            return const_cast<cli::Cli*>(pcli_Cli);
        }
    }

    return NULL;
}

cli::Shell* const GetShell(void)
{
    if (const cli::Cli* const pcli_Cli = GetCli())
    {
        if (cli::Shell* const pcli_Shell = & pcli_Cli->GetShell())
        {
            return pcli_Shell;
        }
    }

    return NULL;
}

int SendToAtbmd(std::string strCommand)
{
    char command[1600];
	struct sockaddr_un ser_un;
	int tmp_argc = 1;
	int socket_fd = 0;
	int ret = 0;
	int i = 0;

    memset(command, 0, sizeof(command));
    strncpy(command,strCommand.c_str(),100);

	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_fd <= 0)
	{
        cli::OutputDevice::GetStdErr() << "open socket err" << cli::endl;
		return -1;
	}

	memset(&ser_un, 0, sizeof(ser_un));  
    ser_un.sun_family = AF_UNIX;  
	strcpy(ser_un.sun_path, "/usr/wifi/server_socket");

	ret = connect(socket_fd, (struct sockaddr *)&ser_un, sizeof(struct sockaddr_un));  
	if(ret < 0)  
	{  
        cli::OutputDevice::GetStdErr() << "connect err" << cli::endl;
		return -2; 
	}

	write(socket_fd, command, strlen(command)+1);
	read(socket_fd, command, sizeof(command));
	if (strcmp(command, "OK"))
	{
        cli::OutputDevice::GetStdErr() << "send cmd err" << cli::endl;
		return -3;
	}

    return 0;
}

int CliAgentCommonInit()
{
    AiInterface = new CZeratulAudioInterface("ai");
    AoInterface = new CZeratulSpeakerInterface("ao");

    if(AiInterface == NULL)
    {
        cli::OutputDevice::GetStdErr() << "AiInterface:new failure" << cli::endl;
        return -1;
    }

    AiInterface->init();

    if(AoInterface == NULL)
    {
        cli::OutputDevice::GetStdErr() << "AoInterface:new failure" << cli::endl;
        return -1;
    }

    AoInterface->init();

    return 0;
}

int AudioRecord(const char *FileName, int FrameNum)
{
    int fd = open(FileName,O_WRONLY | O_CREAT);
    if(fd <= 0)
    {
        cli::OutputDevice::GetStdErr() << "AudioRecord:open failure" << cli::endl;
        return -2;
    }

    int ret_size = 0,data_len = 0;
    unsigned char *tmp;

    lseek(fd,SEEK_SET,WAV_HEAD_LEN);
    for(int i = 0;i < FrameNum;i++)
    {
       tmp = AiInterface->readFrame(0,&ret_size);
       write(fd,tmp,ret_size);
       data_len += ret_size;
    }

    //4-7 文件长度
    //最后4个字节，数据长度
    unsigned char wav_head[WAV_HEAD_LEN] = 
    {
        0x52,0x49,0x46,0x46,0xFC,0xFD,0xFE,0xFF,0x57,0x41,0x56,0x45,0x66,0x6d,0x74,0x20,
        0x12,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x80,0x3e,0x00,0x00,0x00,0x7d,0x00,0x00,
        0x02,0x00,0x10,0x00,0x00,0x00,0x4c,0x49,0x53,0x54,0x1a,0x00,0x00,0x00,0x49,0x4e,
        0x46,0x4f,0x49,0x53,0x46,0x54,0x0e,0x00,0x00,0x00,0x4c,0x61,0x76,0x66,0x35,0x38,
        0x2e,0x32,0x39,0x2e,0x31,0x30,0x30,0x00,0x64,0x61,0x74,0x61,0xFC,0xFD,0xFE,0xFF
    };

    wav_head[WAV_HEAD_LEN - 1] = (data_len >> 24) & 0xFF;
    wav_head[WAV_HEAD_LEN - 2] = (data_len >> 16) & 0xFF;
    wav_head[WAV_HEAD_LEN - 3] = (data_len >> 8) & 0xFF;
    wav_head[WAV_HEAD_LEN - 4] = data_len & 0XFF;

    data_len += WAV_HEAD_LEN;
    wav_head[7] = (data_len >> 24) & 0xFF;
    wav_head[6] = (data_len >> 16) & 0xFF;
    wav_head[5] = (data_len >> 8) & 0xFF;
    wav_head[4] = data_len & 0XFF;

    lseek(fd,SEEK_SET,0);
    write(fd,wav_head,WAV_HEAD_LEN);

    close(fd);

    return 0;
}

int AudioAplay(const char *FileName)
{
    int fd = open(FileName,O_RDONLY);
    if(fd <= 0)
    {
        cli::OutputDevice::GetStdErr() << "AudioAplay:open failure" << cli::endl;
        return -2;
    }

    int ret_size = 0;
    unsigned char tmp[AUDIO_FRAME_LEN];

    AoInterface->resumeChn();
    lseek(fd,SEEK_SET,WAV_HEAD_LEN);
    do
    {
       ret_size = read(fd,tmp,AUDIO_FRAME_LEN);
       
       if(ret_size != AUDIO_FRAME_LEN)
       {
            break;
       }
       
       AoInterface->writeFrame(0,tmp,ret_size);

    }while(1);

    AoInterface->pauseChn();
    AoInterface->clearChnBuf();

    close(fd);

    return 0;
}

int ReadFile(const char* FileName,char** FileData,int *FileLength)
{
    int fd = open(FileName,O_RDONLY);

    int size = lseek(fd,0,SEEK_END);

    *FileData = new char[size + 1];

    int ret = 0,read_len = 0;
    
    lseek(fd,0,SEEK_SET);

    do
    {
        ret = read(fd,*FileData + read_len,size - read_len);
        if(ret <= 0)
        {
            break;
        }
        read_len += ret;
    }while(read_len == size);

    *FileLength = read_len;
    *FileData[read_len] = 0;

    cli::OutputDevice::GetStdErr() << "WCQ" << read_len << "  size" << size << cli::endl;

    return read_len;
}

void SetTaskStatus(int NameStatus)
{
    int fd = open("/system/demo", O_WRONLY|O_CREAT);
    char buf = 1;
    if (fd < 0)
    {
        lseek(fd,NameStatus,SEEK_SET);
        write(fd,"1",1);
        close(fd);
    }
}

int GetOOB(char* oob)
{
    int ret = -1;
    int connetErrno = 0;

    std::string strCommond = "{\"method\":\"local.query_qr_code\",\"params\":\"\",\"id\":123456}";
    struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(54322);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	int sockClient = socket(AF_INET, SOCK_STREAM, 0);

    struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sockClient, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
	setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

	ret = connect(sockClient, (struct sockaddr*)&server, sizeof(struct sockaddr));
    if (ret)
	{
		connetErrno = errno;
		printf("connect error: %s\n", strerror(connetErrno));
		return -1;
	}
	else
	{
		printf("Connect successful!\n");
	}

    ret = send(sockClient, strCommond.c_str(), strCommond.length(), 0);
	if (ret == -1)
	{
		connetErrno = errno;
		printf("send error: %s\n", strerror(connetErrno));
		return -1;
	}

    //char recvbuf[1024] = { 0 };
	ret = recv(sockClient, oob, sizeof(oob), 0);
	if (ret == -1)
	{
        connetErrno = errno;
        printf("recv error: %s\n", strerror(connetErrno));
        return -1;
	}
	//printf("receiveMsg: %d, recvbuf: %s \n", ret, oob);

    return 0;
}