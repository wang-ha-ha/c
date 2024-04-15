#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <Eigen/Dense>
#include "common.h"
#include "fw_env_para.h"
#include <sys/mount.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
// #define _TIME_DEBUG

#ifdef _TIME_DEBUG
#include <chrono>
#endif  // _TIME_DEBUG

using namespace std;

using EigenMatrix = Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

bool m_bFrame1 = false;
bool m_bFrame2 = false;

EigenMatrix grayImage1;
EigenMatrix grayImage2;

int iConsecutivePositiveDiffCount = 0;
bool consecutivePositiveDiff = false;
double totalTime = 0.0;

#define YUV_FRAME_SIZE 		            460800      //YUV size 640*480*1.5
#define FRAME_NUM_EIGEN 	            3           //frame rate
#define MAX_PIXEL_COUNT                 30          //max pixef count

#define _RINGBUF_LEN 		4
typedef struct _ringbuf_t_
{
    volatile unsigned int   ringbuf_head;
    volatile unsigned int   ringbuf_tail;
    volatile unsigned char  p_ringbuf[_RINGBUF_LEN][YUV_FRAME_SIZE];
}ringbuf;

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    /*获取原有串口配置*/
    if  ( tcgetattr( fd,&oldtio)  !=  0) { 
        perror("SetupSerial 1");
        return -1;
    }
    memset( &newtio, 0, sizeof(newtio) );
    /*CREAD 开启串行数据接收，CLOCAL并打开本地连接模式*/
    newtio.c_cflag  |=  CLOCAL | CREAD;

    /*设置数据位*/
    newtio.c_cflag &= ~CSIZE;
    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }
    /* 设置奇偶校验位 */
    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E': 
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':  
        newtio.c_cflag &= ~PARENB;
        break;
    }
    /* 设置波特率 */
    switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    /*设置停止位*/
    if( nStop == 1 )/*设置停止位；若停止位为1，则清除CSTOPB，若停止位为2，则激活CSTOPB*/
        newtio.c_cflag &=  ~CSTOPB;/*默认为一位停止位； */
    else if ( nStop == 2 )
        newtio.c_cflag |=  CSTOPB;
    newtio.c_iflag &= ~(IXON);
    /*设置最少字符和等待时间，对于接收字符和等待时间没有特别的要求时*/
    newtio.c_cc[VTIME]  = 0;/*非规范模式读取时的超时时间；*/
    newtio.c_cc[VMIN] = 0;/*非规范模式读取时的最小字符数*/
    /*tcflush清空终端未完成的输入/输出请求及数据；TCIFLUSH表示清空正收到的数据，且不读取出来 */
    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
//  printf("set done!\n\r");
    return 0;
}

// 自定义帧差计算
EigenMatrix selfabsdiff(const EigenMatrix& src1, const EigenMatrix& src2)
{
    if (src1.size() != src2.size())
    {
        std::cerr << "Input images must have the same size." << std::endl;
        return EigenMatrix();
    }

    return (src1.cast<int>() - src2.cast<int>()).cwiseAbs().cast<uint8_t>();
}

// 自定义膨胀
void customDilation(const EigenMatrix& input, EigenMatrix& output, const EigenMatrix& kernel)
{
    int kernelRows = kernel.rows();
    int kernelCols = kernel.cols();
    int halfKernelRows = kernelRows / 2;
    int halfKernelCols = kernelCols / 2;

    output = EigenMatrix::Zero(input.rows(), input.cols());

    for (int i = halfKernelRows; i < input.rows() - halfKernelRows; ++i)
    {
        for (int j = halfKernelCols; j < input.cols() - halfKernelCols; ++j)
        {
            auto maxVal = input.block<3, 3>(i - halfKernelRows, j - halfKernelCols).maxCoeff();
            output(i, j) = maxVal;
        }
    }
}

// 自定义腐蚀
void customErosion(const EigenMatrix& input, EigenMatrix& output, const EigenMatrix& kernel)
{
    // Assuming kernel is odd-sized and square
    int kernelRows = kernel.rows();
    int kernelCols = kernel.cols();
    int halfKernelRows = kernelRows / 2;
    int halfKernelCols = kernelCols / 2;

    output = EigenMatrix::Zero(input.rows(), input.cols());

    // Iterate over the image (excluding the boundaries)
    for (int i = halfKernelRows; i < input.rows() - halfKernelRows; ++i)
    {
        for (int j = halfKernelCols; j < input.cols() - halfKernelCols; ++j)
        {
            int minVal = 255;  // Start with the maximum value for unsigned char

            // Iterate over the kernel
            for (int x = -halfKernelRows; x <= halfKernelRows; ++x)
            {
                for (int y = -halfKernelCols; y <= halfKernelCols; ++y)
                {
                    // Only consider the pixel if the kernel value at this position is non-zero
                    if (kernel(x + halfKernelRows, y + halfKernelCols))
                    {
                        int pixelValue = input(i + x, j + y);
                        minVal = std::min(pixelValue, minVal);
                        if (minVal == 0)
                        {
                            goto done;
                        }
                    }
                }
            }
        done:
            output(i, j) = minVal;  // Set the output pixel
        }
    }
}

bool eigenIdentify(unsigned char* frameBuff, int frameSize, bool &bResult)
{
    int width = 640; // 图像宽度
    int height = 480; // 图像高度

    int currentframeSize = width * height * 3 / 2; // YUV NV12格式的帧大小
    if(currentframeSize != frameSize)
    {
        printf( "eigenIdentify frameSize error, currentframeSize %d != frameSize %d\n", currentframeSize, frameSize);
        return false;
    }

    if (frameBuff == NULL)
    {
        printf( "eigenIdentify frameBuff is NULL\n");
        return false;
    }

    // 读第一帧
    if (!m_bFrame1)
    {
        printf( "eigenIdentify set first frame...\n");

        grayImage1.setZero(height, width);
        std::memcpy(grayImage1.data(), frameBuff, width * height);

        m_bFrame1 = true;
        return true;
    }

    // 读第二帧
    if (!m_bFrame2)
    {
        printf( "eigenIdentify set second frame...\n");

        grayImage2.setZero(height, width);
        std::memcpy(grayImage2.data(), frameBuff, width * height);

        m_bFrame2 = true;
        return true;
    }

    // 循环读后续帧
    printf( "eigenIdentify set current frame...\n");

#ifdef _TIME_DEBUG
    // 开始计时
    auto startTime = std::chrono::high_resolution_clock::now();
    auto startTime1 = std::chrono::high_resolution_clock::now();
#endif  // _TIME_DEBUG

    EigenMatrix grayImage(height, width);
    std::memcpy(grayImage.data(), frameBuff, width * height);

#ifdef _TIME_DEBUG
    auto endTime1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration1 = endTime1 - startTime1;
    totalTime = duration1.count();
    printf( "YUV转灰度时间: %f 毫秒\n", totalTime * 1000);

    auto startTime2 = std::chrono::high_resolution_clock::now();
#endif  // _TIME_DEBUG

    // 计算三帧帧差
    auto self_frame_diff1 = selfabsdiff(grayImage2, grayImage1);
    auto self_frame_diff2 = selfabsdiff(grayImage, grayImage2);

    // TODO 这里逻辑有问题，先按原逻辑来
    // matFrameDiff = self_frame_diff1 | self_frame_diff2;
    EigenMatrix frame_diff = self_frame_diff1.binaryExpr(self_frame_diff2, std::bit_or<uint8_t>());
    std::vector<std::vector<int>> kernelErosion = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

    // 开操作
    EigenMatrix eigenFilteredFrameDiff1, eigenFilteredFrameDiff;
    EigenMatrix eigenKernel = EigenMatrix::Ones(3, 3);

#ifdef _TIME_DEBUG
    auto endTime2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration2 = endTime2 - startTime2;
    totalTime = duration2.count();
    printf( "帧差计算时间: %f 毫秒\n", totalTime * 1000);

    auto startTime3 = std::chrono::high_resolution_clock::now();
#endif  // _TIME_DEBUG

    // 腐蚀
    customErosion(frame_diff, eigenFilteredFrameDiff1, eigenKernel);
    // 膨胀
    customDilation(eigenFilteredFrameDiff1, eigenFilteredFrameDiff, eigenKernel);

#ifdef _TIME_DEBUG
    auto endTime3 = std::chrono::high_resolution_clock::now();
    totalTime = std::chrono::duration<double>(endTime3 - startTime3).count();
    printf("eigen 腐蚀膨胀时间: %f 毫秒\n", totalTime * 1000);

    auto startTime4 = std::chrono::high_resolution_clock::now();
#endif  // _TIME_DEBUG

    int pixelCount = (eigenFilteredFrameDiff.array() > MAX_PIXEL_COUNT).count();

#ifdef _TIME_DEBUG
    printf("eigen count: %d\n", pixelCount);
    auto endTime4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration4 = endTime4 - startTime4;
    totalTime = duration4.count();
    printf("countZero时间: %f 毫秒\n", totalTime * 1000);
#endif  // _TIME_DEBUG

    if (pixelCount > 0)
    {
        iConsecutivePositiveDiffCount++;

        // 判断连续帧差是否都大于0
        if (iConsecutivePositiveDiffCount >= (FRAME_NUM_EIGEN - 2) )
        {
            printf("eigenIdentify Recognize changes in the screen!\n");
            bResult = true;
            return true;
        }
    }
    else
    {
        iConsecutivePositiveDiffCount = 0;
    }

    // 更新前两帧
    grayImage1 = grayImage2;

    // 更新第二帧
    grayImage2 = grayImage;

    // 图像延迟
    // chrono::duration<double> delay(0.05);
    // this_thread::sleep_for(delay);

    // iCountframe++;
    // if (bResult)  // && (!countframe))
    // {
    // 	logPrint(MX_LOG_DEBUG, "第[%d]帧开始为 1", iCountframe);
    // }

#ifdef _TIME_DEBUG
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;
    totalTime = duration.count();
    std::cout << "运行时间: " << totalTime * 1000 << " 毫秒" << std::endl;
#endif  // _TIME_DEBUG

    return true;
}

#define ATBM_IOCTL          (121)
#define ATBM_FW_SLEEP             _IO(ATBM_IOCTL, 31)
#define ATBM_PRE_RMMOD            _IO(ATBM_IOCTL, 38)
#define ATBM_TX_EMPTY             _IOR(ATBM_IOCTL,39, unsigned int)

void enter_lowpower()
{
    char *uart = "/dev/ttyS0";
    char cmd[128] = {0};
    int fd;

    sprintf(cmd,"enter_lowpower\r\n");

    if((fd = open(uart, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
    {
        printf("enter_lowpower open %s is failed",uart);
    }
    else
    {
        printf("enter_lowpower open %s is success\r\n",uart);
        set_opt(fd, 115200, 8, 'N', 1); 
        write(fd,cmd,strlen(cmd) + 1);
        close(fd);
    }
}

int main()
{
    // system("echo low    > /sys/class/gpio/gpio45/direction");
    int iCount = 0;
    int iFrameThreshold = 0;
    bool bEigenResult = false;

    ringbuf *rb;

    int shm_id;
    key_t shm_key;


    while(1)
    {
        if ((shm_key = ftok("/etc/passwd", 12345)) == -1) {
            perror("ftok()");
            goto done;
        }

        shm_id = shmget(shm_key, sizeof(ringbuf), 0);
        if (shm_id < 0) {
            // perror("shmget()");
            // exit(1);
            usleep(10*1000);
            continue;
        }
        break;
    }

    rb = (ringbuf *)shmat(shm_id, NULL, 0);
    if ((void *)rb == (void *)-1) {
        perror("shmat()");
        goto done;
    }
    // system("echo low    > /sys/class/gpio/gpio45/direction");
    while (1)
    {
        //printf("rapid_identify rb->ringbuf_tail:%d, rb->ringbuf_head:%d\n",rb->ringbuf_tail,rb->ringbuf_head);
        while(rb->ringbuf_tail == rb->ringbuf_head)
        {
            usleep(100*1000);
        }

        iFrameThreshold++;

        if(!bEigenResult)
        {
            eigenIdentify((unsigned char*)rb->p_ringbuf[rb->ringbuf_head], YUV_FRAME_SIZE, bEigenResult);
        
            if(iFrameThreshold >= FRAME_NUM_EIGEN && !bEigenResult) 
            {
                // system("echo high    > /sys/class/gpio/gpio45/direction");
                //小算法在使用未识别到画面变动的情况下，直接进低功耗
                printf("eigenIdentify not recognize changes, enter lowpower...\n");

                int iWakeupCnt = 0;
                while(1)
                {
                    if(access("/tmp/616_info",F_OK) == 0)
                    {
                       break;
                    }
                    if (iWakeupCnt++ >= 50)
                    {
                        goto done;
                    }
                    usleep(10*1000);
                }

                if(getPowerOnTypeValue() != WAKEUP_PIR)
                {
                    goto done;
                }

                usleep(100*1000);
                if(access("/tmp/resetdev", F_OK) == 0)
                {
                    goto done;
                }

                remove("/var/daemon/log_manage_subsystem");
                remove("/var/daemon/mike_manage_subsystem");
                // printf("apk_enter_lowpower a 6\n");
                sync();
                system("killall log_manage_subsystem");
                system("killall mike_manage_subsystem");
                system("killall agent");
                // printf("apk_enter_lowpower a 7\n");
                system("df -h /userfs");
                // printf("apk_enter_lowpower a 8\n");
                int ret = umount2("/userfs", MNT_FORCE);
                if (ret != 0) {
                    printf("umount2 fail, ret=%d, errno=%s\n", ret, strerror(errno));
                }
                // printf("apk_enter_lowpower a 9\n");
                sync();

                printf("apk_enter_lowpower \n");
                int wait_time = 5;
                while(wait_time > 0)
                {
                    enter_lowpower();
                    wait_time--;
                    usleep(300 * 1000);
                }

                return 0;
            }
        }
        else
        {
            printf("eigenIdentify recognize changes, exit self...\n");
            goto done;
        }

        rb->ringbuf_head = (rb->ringbuf_head + 1) % _RINGBUF_LEN;
    }

done:
    createFile("/tmp/_runSystemBin");
	return 0;
}
