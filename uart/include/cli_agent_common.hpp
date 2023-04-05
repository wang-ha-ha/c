#ifndef __CLI_AGENT_COMMON_HPP__
#define __CLI_AGENT_COMMON_HPP__
#include "cli/pch.h"
#include "cli/common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>  
#include <errno.h>  
#include <arpa/inet.h>  
#include <signal.h>
#include <stdarg.h>
#include <fstream>

#include "zeratul_audio_interface.h"
#include "zeratul_speaker_interface.h"

#define UART_PATH "/dev/ttyS2"
#define MCU_TYPE  "atbm6441"

#define AUDIO_FRAME_LEN 1280  // 一帧1280个字节
#define AUDIO_FRAME_NUM 75    // 1S 25帧，存3S
#define WAV_HEAD_LEN    0x50  // wav 头部信息长度

extern CZeratulAudioInterface *AiInterface;
extern CZeratulSpeakerInterface *AoInterface;

extern int CliAgentCommonInit();
extern int SendToAtbmd(std::string strCommand);
extern int AudioRecord(const char *FileName, int FrameNum);
extern int AudioAplay(const char *FileName);
extern int ReadFile(const char* FileName,char** FileData,int *FileLength);
extern void SetTaskStatus(int NameStatus);
extern int GetOOB(char* oob);

#endif //__CLI_AGENT_COMMON_HPP__
