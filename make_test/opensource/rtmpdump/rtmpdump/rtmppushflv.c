/*  RTMPDump
 *  Copyright (C) 2009 Andrej Stepanchuk
 *  Copyright (C) 2009 Howard Chu
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RTMPDump; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#define _FILE_OFFSET_BITS	64

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include <signal.h>		// to catch Ctrl-C
#include <getopt.h>

#include "librtmp/rtmp_sys.h"
#include "librtmp/log.h"


#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00))
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
	(x<<8&0xff0000)|(x<<24&0xff000000))
#define HTONTIME(x) ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00)|(x&0xff000000))

/*read 1 byte*/
int ReadU8(unsigned int *u8,FILE*fp)
{
	uint8_t tmp[1];
	if(fread(tmp,1 ,1 ,fp)!=1)
		return 0;
	*u8 = tmp[0];
	return 1;
}
/*read 2 byte*/
int ReadU16(unsigned int *u16,FILE*fp)
{
	uint8_t tmp[2];
	if(fread(tmp ,1 , 2, fp)!=2)
		return 0;

	*u16 = tmp[0]<<8|tmp[1];
	return 1;
}
/*read 3 byte*/
int ReadU24(unsigned int *u24,FILE*fp)
{
	uint8_t tmp[3];
	if(fread(tmp, 1, 3, fp)!=3)
		return 0;
	*u24=tmp[0]<<16|tmp[1]<<8|tmp[2];
	return 1;
}
/*read 4 byte*/
int ReadU32(unsigned int *u32,FILE*fp)
{
	uint8_t tmp[4];
	if(fread(tmp, 1, 4 ,fp )!=4)
		return 0;
	*u32=tmp[0]<<24|tmp[1]<<16|tmp[2]<<8|tmp[3];
	return 1;
}
/*read 1 byte,and loopback 1 byte at once*/
int PeekU8(unsigned int *u8,FILE*fp)
{
	if(fread(u8, 1, 1, fp)!=1)
		return 0;
	fseek(fp,-1,SEEK_CUR);
	return 1;
}
/*read 4 byte and convert to time format*/
int ReadTime(unsigned int *utime,FILE*fp)
{
	uint8_t tmp[4];
	if(fread(tmp, 1, 4 , fp)!=4)
		return 0;
	*utime=tmp[0]<<16|tmp[1]<<8|tmp[2]|tmp[3]<<24;
	return 1;
}

void publish_flvfile(RTMP *rtmp, char *filename)
{
	int ret;
	RTMPPacket *packet;
	unsigned int preTagsize;
	int i;

	

	//packet attributes
	unsigned int type;
	unsigned int datalength;
	unsigned int timestamp;
	unsigned int streamid;
	FILE *fp;

	packet=(RTMPPacket*)malloc(sizeof(RTMPPacket));
	RTMPPacket_Alloc(packet,1024*1024);
	RTMPPacket_Reset(packet);

	fp = fopen(filename, "r");
	if(!fp)
	{
		printf("open file success \n");
	}else
	{
		printf("open file failed\n");
	}


	RTMP_LogPrintf("Start to send data ...\n");

	//jump over FLV Header
	//jump over previousTagSizen
	fseek(fp,9+4,SEEK_SET);
	i=0;
	while(1)
	{
		i++;
		//not quite the same as FLV spec
		if(!ReadU8(&type,fp))
		{
			RTMP_Log(RTMP_LOGERROR,"read type failed\n");
			break;
		}

		if(!ReadU24(&datalength,fp))
		{
			RTMP_Log(RTMP_LOGERROR,"read datalength failed\n");
			break;
		}

		if(datalength == 0)
		{
			RTMP_Log(RTMP_LOGERROR,"read datalength == 0\n");
			break;
		}		

		if(!ReadTime(&timestamp,fp))
		{
			RTMP_Log(RTMP_LOGERROR,"read timestamp failed\n");
			break;
		}

		if(!ReadU24(&streamid,fp))
		{
			RTMP_Log(RTMP_LOGERROR,"read streamid failed\n");
			break;
		}
		
		printf("id %d: type %d, len %d, timestamp %d, streamid %d\n",
				i, type,datalength, timestamp, streamid);

		if (type!=0x08 && type!=0x09){
			//jump over non_audio and non_video frame��
			//jump over next previousTagSizen at the same time
			fseek(fp,datalength+4,SEEK_CUR);
			continue;
		}

		if(fread(packet->m_body, 1, datalength, fp )!=datalength)
		{
			RTMP_Log(RTMP_LOGERROR,"read data failed\n");
			break;
		}
		packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet->m_nTimeStamp = timestamp;
		packet->m_packetType = type;
		packet->m_nBodySize  = datalength;

		packet->m_hasAbsTimestamp = 0;
		packet->m_nChannel = 0x04;
		packet->m_nInfoField2 = rtmp->m_stream_id;

		if (!RTMP_IsConnected(rtmp)){
			RTMP_Log(RTMP_LOGERROR,"rtmp is not connect\n");
			break;
		}

		ret = RTMP_SendPacket(rtmp, packet, 0);

		if (!ret){
			RTMP_Log(RTMP_LOGERROR,"Send Error\n");
			break;
		}

		if(!ReadU32(&preTagsize,fp))
		{
			RTMP_Log(RTMP_LOGERROR,"read preTagsize failed\n");
			break;
		}
	}

	RTMP_LogPrintf("\nSend Data Over\n");


	if (packet!=NULL){
		RTMPPacket_Free(packet);
		packet=NULL;
	}


	fclose(fp);
}



#define DEF_TIMEOUT	30	/* seconds */
#define DEF_BUFTIME	(10 * 60 * 60 * 1000)	/* 10 hours default */

int rtmp_dump(char *url)
{ 
	int ret;
	RTMP rtmp;
	int now, lastUpdate;
	unsigned int bufferTime = DEF_BUFTIME;
	int bufferSize = 64*1024;;
	char* buffer;
	unsigned int recvsize;
	int nRead;

	buffer = (char *) malloc(bufferSize);

  	RTMP_debuglevel = RTMP_LOGINFO;

	RTMP_Log(RTMP_LOGINFO, "RTMP_Init...\n");
	RTMP_Init(&rtmp);
	RTMP_LogPrintf("RTMP_SetupStream ...\n");
    if (RTMP_SetupURL(&rtmp, url) == FALSE)
    {
		RTMP_Log(RTMP_LOGERROR, "Couldn't parse URL: %s", url);
		return -1;
	}

	RTMP_Log(RTMP_LOGDEBUG, "Setting buffer time to: %dms", bufferTime);
	RTMP_SetBufferMS(&rtmp, bufferTime);
	RTMP_LogPrintf("Connecting ...\n");

	ret = RTMP_Connect(&rtmp, NULL);

	if (!ret)
	{
		RTMP_Log(RTMP_LOGINFO, "Connecte failed\n");
		goto exit;
	}

	RTMP_Log(RTMP_LOGINFO, "Connected...\n");

	RTMP_ConnectStream(&rtmp, 0);
	if (!ret)
	{
		RTMP_Log(RTMP_LOGINFO, "Connecte Stream failed\n");
		goto exit;
	}

	RTMP_LogPrintf("Starting download...\n");
	now = RTMP_GetTime();
	lastUpdate = now - 1000;
	while(1)
	{
		nRead = RTMP_Read(&rtmp, buffer,  bufferSize);
		recvsize += nRead;

		now = RTMP_GetTime();
		buffer[10] = 0;
		printf("%s\n", buffer);
		if (abs(now - lastUpdate) > 1000)
		{
			RTMP_LogPrintf("**************************nRead: %dkbs\n", recvsize*8/1024);
			recvsize = 0;
			lastUpdate = now;
		}
	}

exit:
	RTMP_Log(RTMP_LOGDEBUG, "Closing connection.\n");
	RTMP_Close(&rtmp);
	free(buffer);
	return 0;
} 



int rtmp_upload(char *url)
{
	int ret;
	RTMP rtmp;
	unsigned int bufferTime = DEF_BUFTIME;

	RTMP_Log(RTMP_LOGINFO, "RTMP_Init...\n");
	RTMP_Init(&rtmp);
	RTMP_LogPrintf("RTMP_SetupStream ...\n");
    if (RTMP_SetupURL(&rtmp, url) == FALSE)
    {
		RTMP_Log(RTMP_LOGERROR, "Couldn't parse URL: %s", url);
		return -1;
	}

	RTMP_EnableWrite(&rtmp);

	RTMP_Log(RTMP_LOGDEBUG, "Setting buffer time to: %dms", bufferTime);
	RTMP_SetBufferMS(&rtmp, bufferTime);
	RTMP_LogPrintf("Connecting ...\n");

	ret = RTMP_Connect(&rtmp, NULL);

	if (!ret)
	{
		RTMP_Log(RTMP_LOGINFO, "Connect failed\n");
		goto exit;
	}

	RTMP_Log(RTMP_LOGINFO, "Connected...\n");


	ret = RTMP_ConnectStream(&rtmp, 0);
	if (!ret)
	{
		RTMP_Log(RTMP_LOGINFO, "Connecte Stream failed\n");
		goto exit;
	}

	RTMP_LogPrintf("Starting upload...\n");
	while(1)
		publish_flvfile(&rtmp, "1.flv");

exit:
	RTMP_Log(RTMP_LOGDEBUG, "Closing connection.\n");
	RTMP_Close(&rtmp);
	return 0;
}




int main(int argc, char **argv)
{
	if(argc <= 1)
	{
		return -1;
	}
	if( !strcmp(argv[1],"upload") )
	{
		rtmp_upload("rtmp://139.196.214.42:1935/rtmp/1");
	}else if( !strcmp(argv[1],"dump") )
	{
		rtmp_dump("rtmp://58.200.131.2:1935/livetv/cctv1");
	}
	return 0;
}
