#include "librtmp_send264.h"
#include <iostream>
#include <string>

using namespace std;

int main() {

	CRTMPStream *mCRTMPStream = new CRTMPStream();

	string rtmpAddress = "rtmp://172.17.151.72:1935/live/movie";

	// string videoName = "./test.264";	
	string videoName = "./720p.h264.raw";
	string videoDir = "./h264_bitstream/";

	if(mCRTMPStream->Connect(rtmpAddress.c_str()) > 0) {
		cout<<"Connect to "<< rtmpAddress <<" successfully!" <<endl;
	} else {
		cout<<"Connect to "<< rtmpAddress <<" failed!" <<endl;
		exit(1);
	}

#if 1
	while(1){
		if(mCRTMPStream->SendH264File(videoName.c_str())<0) {
			cout << "Send x264 file failed!" <<endl;
		}
	}
#else
	while(1){
		mCRTMPStream->SendH264Frames(videoDir.c_str());
		Sleep(10);
	}
#endif

}
