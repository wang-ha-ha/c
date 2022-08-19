all:mp4_info Demuxing

mp4_info:mp4_info.o
	g++ -g -o mp4_info mp4_info.cpp -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lx264

Demuxing:Demuxing.o Demuxing_test.o
	g++ -g -o Demuxing Demuxing.cpp Demuxing_test.cpp -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lx264

clean:
	rm -f *.o mp4_info Demuxing