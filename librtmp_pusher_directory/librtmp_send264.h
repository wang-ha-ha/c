#pragma once
#include <vector>
#include <string>
#include <iostream>


#include "rtmp.h"
#include "rtmp_sys.h"
#include "amf.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

#define FILEBUFSIZE (1024 * 1024 * 10)       //  10M

// NALU��Ԫ
typedef struct _NaluUnit
{
	int type;
	int size;
	unsigned char *data;
}NaluUnit;

typedef struct _RTMPMetadata
{
	// video, must be h264 type
	unsigned int	nWidth;
	unsigned int	nHeight;
	unsigned int	nFrameRate;		// fps
	unsigned int	nVideoDataRate;	// bps
	unsigned int	nSpsLen;
	unsigned char	Sps[1024];
	unsigned int	nPpsLen;
	unsigned char	Pps[1024];

	// audio, must be aac type
	bool	        bHasAudio;
	unsigned int	nAudioSampleRate;
	unsigned int	nAudioSampleSize;
	unsigned int	nAudioChannels;
	char		    pAudioSpecCfg;
	unsigned int	nAudioSpecCfgLen;

} RTMPMetadata,*LPRTMPMetadata;


class CRTMPStream
{
public:
	CRTMPStream(void);
	~CRTMPStream(void);
public:
	// ���ӵ�RTMP Server
	bool Connect(const char* url);
    // �Ͽ�����
	void Close();
    // ����MetaData
	bool SendMetadata(LPRTMPMetadata lpMetaData);
    // ����H264����֡
	bool SendH264Packet(unsigned char *data,unsigned int size,bool bIsKeyFrame,unsigned int nTimeStamp);
	// ����H264�ļ�
	bool SendH264File(const char *pFileName);
	// send H264 data frames
	bool SendH264Frames(const char *pFrameDataDir);

private:
	// �ͻ����ж�ȡһ��NALU��
	bool ReadOneNaluFromBuf(NaluUnit &nalu);
	// ��������
	int SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp);
	// get H264 frames number
	int getFramesSum(const char *pFrameDataDir);
	// process first frame, get sps, psp, sei, idr
	vector<NaluUnit*> processFirstFrame(unsigned char *data, const unsigned int length);
	// get frame data from file
	int getFrameData(string fileName, unsigned char *frameBuffer, unsigned int frameBufferSize);
	// process frames excluding first frame
	NaluUnit processNormalFrame(unsigned char *data, const unsigned int length);

private:
	RTMP* m_pRtmp;
	unsigned char* m_pFileBuf;
	unsigned int  m_nFileBufSize;
	unsigned int  m_nCurPos;
};
