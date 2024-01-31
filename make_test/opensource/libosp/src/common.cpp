#include "common.h"
#include <string>
#include <vector>
#include <fstream>
#include <time.h>
#ifdef _WIN32
#include <io.h>
#include <time.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <tlhelp32.h>
#else
#include <dirent.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h> 
#include <unistd.h> 
#include <netdb.h>  
#include <net/if.h>  
#include <arpa/inet.h> 
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <stdarg.h>
#endif
#include "fw_env_para.h"
#include <string.h>
#include <thread>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <stdarg.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// extern "C"{
// #include "unwind_backtrace.h"
// }

mxbool bindCpu(int cpu_id)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpu_id,&mask);
	if(sched_setaffinity(0,sizeof(mask),&mask) == -1)
	{
		printf("warning: could not set CPU affinity\n");
		return mxfalse;
	}	

	return mxtrue;
}

mxbool getFiles(std::string path,
	std::vector<std::string>& files, const char* sType)
{
#ifdef _WIN32
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(),
		&fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && 
					strcmp(fileinfo.name, "..") != 0)
				{
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files, sType);
				}
			}
			else
			{
				char* pName = fileinfo.name;
				char* pFind = strstr(pName, sType);
				if (pFind != NULL)
				{
					files.push_back(p.assign(path).
						append("\\").append(fileinfo.name));
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
#else
    DIR* dir = NULL;
    if ((dir = opendir(path.c_str())) != NULL)
    {
		struct dirent* d_ent = NULL;
        while ( (d_ent = readdir(dir)) != NULL )
        {
			if ( d_ent->d_type == DT_DIR )
			{
				if (strcmp(d_ent->d_name, ".") != 0 && 
				 	strcmp(d_ent->d_name, "..") != 0 )
				{
					getFiles(path + "/" + d_ent->d_name, files, sType);
				}
			}
			else
			{
				if ( NULL != strstr(d_ent->d_name, sType) )
				{
					files.push_back(path + "/" + d_ent->d_name);
				}
			}
        }
        closedir(dir);
    }
#endif

	return mxtrue;
}


mxbool getLocalIPByName(std::string strName, std::string &strIP)
{
#ifdef _WIN32
    bool bFind = false;
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));

	if (pAdapterInfo == NULL)
	{
		return mxfalse;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			return mxfalse;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			if (strName.compare(pAdapter->Description) == 0)
			{
				strIP = pAdapter->IpAddressList.IpAddress.String;
				bFind = true;
				break;
			}

			pAdapter = pAdapter->Next;
		}
	}
	else
	{
		if (pAdapterInfo)
		{
			free(pAdapterInfo);
		}
		return mxfalse;
	}

	if (pAdapterInfo)
	{
		free(pAdapterInfo);
	}
	
	return bFind;
#else
	int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
 
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return mxfalse;
    }
 
    strncpy(ifr.ifr_name, strName.c_str(), IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
 
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return mxfalse;
    }
    
    //printf("interfac: %s, ip: %s\n", strName.c_str(), inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr)); 
	strIP = inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
	//printf("interfac: %s, ip: %s\n", strName.c_str(), strIP.c_str()); 

    close(sd);
    return mxtrue;
#endif

}


mxbool linuxPopenExecCmd(std::string &strOutData, const char * pFormat, ...)
{
    char acBuff[512] ={0};

    va_list ap;
    int ret = -1;
    va_start(ap, pFormat);
    ret = vsprintf(acBuff, pFormat, ap);
    va_end(ap);
#ifndef _WIN32
	FILE *pFile = popen(acBuff, "r");
	if (NULL == pFile)
	{	
		return mxfalse;
	}

	char acValue[512] = {0};
	while(fgets(acValue, sizeof(acValue), pFile) != NULL)
	{
		strOutData += acValue;
	}
	pclose(pFile);
#endif

	return mxtrue;
}

mxbool getGUIDData(std::string & strGUID)
{
	char acGUID[64] = { 0 };
	int iConTime = time(NULL);
	srand(iConTime);
	snprintf(acGUID, sizeof(acGUID),
		"%08X-%04X-%04X-%04X-%04X%04X%04X",
		rand() & 0xffffffff,
		rand() & 0xffff,
		rand() & 0xffff,
		rand() & 0xffff,
		rand() & 0xffff, rand() & 0xffff, rand() & 0xffff
		);

	strGUID = acGUID;
	return  mxtrue;
}

mxbool getKey(std::string &strKey)
{
	char acKey[64] = { 0 };
	int iTime = time(NULL);
	srand(iTime);
	snprintf(acKey, sizeof(acKey),
		"%08X%08X",
		rand() & 0xffffffff,
		rand() & 0xffffffff
		);

	strKey = acKey;
	return  mxtrue;
}

int versionCompare(std::string &oldVersion, std::string &newVersion)
{
    int iOldPos = 0, iNewPos = 0;
    int iOldNum = 0, iNewNum = 0;

    printf("%s [%d]:oldVersion.length() = %d, newVersion.length() = %d\n",
    		__FUNCTION__, __LINE__, oldVersion.length(), newVersion.length());

    while (iOldPos < (int)oldVersion.length() || iNewPos < (int)newVersion.length())
    {
        iOldNum = 0;
        while (iOldPos < (int)oldVersion.length() && oldVersion[iOldPos] != '.')
        {
            iOldNum = iOldNum * 10 + (oldVersion[iOldPos] - '0');
            iOldPos++;
        }

        iNewNum = 0;
        while (iNewPos < (int)newVersion.length() && newVersion[iNewPos] != '.')
        {
            iNewNum = iNewNum * 10 + (newVersion[iNewPos] - '0');
            iNewPos++;
        }

        printf("%s [%d]:iOldNum = %d, iNewNum = %d\n", __FUNCTION__, __LINE__, iOldNum, iNewNum);
        if (iOldNum < iNewNum) {
            return 1;
        } else if (iOldNum > iNewNum) {
            return -1;
        }

        iOldPos++;
        iNewPos++;
    }

    return 0;
}

extern "C" {
	#include "efuse.h"
	int readEfuse(int seg_id, int offset, unsigned char *buf, int len);
	int writeEfuse(int seg_id, int offset, unsigned char *buf, int len, int force);
}

std::string getDID()
{
#ifdef WIN32
	return std::string("667179728");
#else
	char did[DID_LEN + 1] = { 0 };
	if(readEfuse(SCB_DATA, DID_OFFSET, (unsigned char *)did, DID_LEN) == 0){
		std::string str(did);
		return str;
	} else {
		return "";
	}
#endif
}

std::string getPSK()
{
#ifdef WIN32
	return std::string("XXQKbV51INfatj3f");
#else
	char psk[PSK_LEN + 1] = { 0 };
	if(readEfuse(USER_DATA, PSK_OFFSET, (unsigned char *)psk, PSK_LEN) == 0){
		std::string str(psk);
		return str;
	} else {
		return "";
	}
#endif
}

std::string getMAC()
{
#ifdef WIN32
	return std::string("7CC294FF3A2A");
#else
	char mac[MAC_LEN + 1] = { 0 };
	if(readEfuse(SCB_DATA, MAC_OFFSET, (unsigned char *)mac, MAC_LEN) == 0){
		std::string str(mac);
		return str;
	} else {
		return "";
	}
#endif
}

int setDID(std::string strDid)
{
	return writeEfuse(SCB_DATA, DID_OFFSET, (unsigned char *)strDid.c_str(), DID_LEN, 1);
}

int setPSK(std::string strPsk)
{
	return writeEfuse(USER_DATA, PSK_OFFSET, (unsigned char *)strPsk.c_str(), PSK_LEN, 1);
}

int setMAC(std::string strMac)
{
	return writeEfuse(SCB_DATA, MAC_OFFSET, (unsigned char *)strMac.c_str(), MAC_LEN, 1);
}

// static void signal_handler(int signo)
// {
// 	maix::logPrint(maix::MX_LOG_ERROR, "[backtrace]signal_handler: %d\n", signo);
// 	printf("[backtrace]signal_handler: %d\n", signo);
//     show_backtrace();
//     exit(EXIT_FAILURE);
// }

void getBackTrace()
{
	// struct sigaction sa;
    // sigset_t mask;

    // sigemptyset(&mask);
    // sa.sa_handler = signal_handler;
    // sa.sa_mask = mask;
    // sa.sa_flags = 0;
    // sigaction(SIGSEGV, &sa, NULL);
    // sigaction(SIGABRT, &sa, NULL);
	// sigaction(SIGBUS, &sa, NULL);
}

int createFile(const char* filename) 
{
    int fd = ::open(filename, O_CREAT | O_WRONLY | O_TRUNC, "0644");
    if (fd == -1) {
        return -1;
    }
	::close(fd);
    return fd;
}

bool deleteFile(const char* filename) 
{
    if (::remove(filename) != 0) {
        return false;
    }
    return true;
}
