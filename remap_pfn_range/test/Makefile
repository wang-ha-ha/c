ifneq ($(CPU),OA8000)
OUT:=out/x86
CC:=gcc
else #make CPU=OA8000
OUT:=out/oa8000
CC:=/opt/arm-ov-linux-20210322/bin/arm-linux-gcc
endif

default:wcq_test_1 wcq_test_2 wcq_test_3

wcq_test_1:test_1.c
	$(CC) $^ -o $@

wcq_test_2:test_2.c
	$(CC) $^ -o $@

wcq_test_3:test_3.c
	$(CC) $^ -o $@

clean:
	rm -f wcq_test_*