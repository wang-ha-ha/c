#include "cli_agent_mcu_io_uart.hpp"
#include "cli/common.h"

namespace maix {
    CCliAgentMcuIoUart::CCliAgentMcuIoUart()
    {
        m_fd = -1;

        m_IoConfig.iBaudRate = 115200;
        m_IoConfig.iBits = 8;
        m_IoConfig.cParity = 'N';
        m_IoConfig.iStop = 1;
        m_IoConfig.iInterval = 10; // ms,当这个时间没有接收到数据代表这一个串口数据报就接收完了；
        m_IoConfig.strUartPath[0] = 0;
    }
    
    CCliAgentMcuIoUart::~CCliAgentMcuIoUart()
    {
        if(m_fd != -1)
        {
            close(m_fd);
        }
    }

    bool CCliAgentMcuIoUart::Configure(T_uMcuIoConfig &Config)
    {
        m_IoConfig.iBaudRate = Config.sUartConfig.iBaudRate;
        m_IoConfig.iBits     = Config.sUartConfig.iBits;
        m_IoConfig.cParity   = Config.sUartConfig.cParity;
        m_IoConfig.iStop     = Config.sUartConfig.iStop;
        m_IoConfig.iInterval = Config.sUartConfig.iInterval;
        strcpy(m_IoConfig.strUartPath,Config.sUartConfig.strUartPath);

        if(Open() == false)
        {
            return false;
        }

        if(SetOpt() == false)
        {
            return false;
        }

        return true;
    }

    int CCliAgentMcuIoUart::SendBuf(unsigned char *pBuf, int iLen)
    {
        if(m_fd < 0)
        {
            return -1;
        }

        int ret = 0;
        int write_len = 0;

        tcflush(m_fd, TCIFLUSH);

        while (iLen > 0) 
        {

            ret = write(m_fd, (char*)pBuf + write_len, iLen);
            if (ret < 1) 
            {
                break;
            }
            write_len += ret;
            iLen = iLen - ret;
        }

        return write_len;
    }

    int CCliAgentMcuIoUart::RecvBuf(unsigned char *pBuf, int iLen, int iTimeout)
    {
        if(m_fd < 0)
        {
            return -1;
        }
        fd_set rset;
        int ret = 0;
        struct timeval t;
        size_t read_len = 0;

        int tims = iTimeout / m_IoConfig.iInterval;

        // cli::OutputDevice::GetStdErr() << "recv Start" << cli::endl;
        while (read_len < iLen)
        {
            FD_ZERO(&rset);
            FD_SET(m_fd, &rset);
            t.tv_sec = 0;
            t.tv_usec = m_IoConfig.iInterval*1000;
            ret = select(m_fd + 1, &rset, NULL, NULL, &t);
            if (ret <= 0) {
                if (errno == EINTR) 
                {
                    // 信号中断
                    cli::OutputDevice::GetStdErr() << "recv EINTR" << cli::endl;
                    continue;
                }

                tims--;
                // 超时（单次接收时间间隔超时）但已经接收到一些数据了
                if (ret == 0 && read_len > 0) 
                {   
                    cli::OutputDevice::GetStdErr() << "recv Done" << cli::endl;
                    break;
                }
                
                // 超出设定接收时间了
                if(tims <= 0)
                {
                    cli::OutputDevice::GetStdErr() << "recv Timeout" << cli::endl;
                    break;
                }

                if(ret < 0)
                {
                    cli::OutputDevice::GetStdErr() << "recv Select Error" << cli::endl;
                    return ret;
                }
            } 
            else
            {
                ret = read(m_fd, (char *)pBuf + read_len, iLen - read_len);
                if (ret < 0)
                {
                    cli::OutputDevice::GetStdErr() << "recv read error" << cli::endl;
                    return ret;
                }
                else
                {
                    read_len += ret;
                }
            }
        }
        // cli::OutputDevice::GetStdErr() << "recv End" << cli::endl;
        return read_len;
    }

    bool CCliAgentMcuIoUart::SetOpt()
    {
        if(m_fd < 0)
        {
            return false;
        }

        struct termios newtio,oldtio;

        if  (tcgetattr(m_fd,&oldtio)  !=  0)
        { 
            cli::OutputDevice::GetStdErr() << "SetupSerial Error" << cli::endl;
            return false;
        }

        memset(&newtio, 0, sizeof(newtio));

        /*CREAD 开启串行数据接收，CLOCAL并打开本地连接模式*/
        newtio.c_cflag  |=  CLOCAL | CREAD;

        /*设置数据位*/
        newtio.c_cflag &= ~CSIZE;
        switch(m_IoConfig.iBits)
        {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
        }

        /* 设置奇偶校验位 */
        switch(m_IoConfig.cParity)
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
        switch(m_IoConfig.iBaudRate)
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
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        }

        /*设置停止位*/
        if(m_IoConfig.iStop == 1)
            newtio.c_cflag &=  ~CSTOPB;
        else if (m_IoConfig.iStop == 2)
            newtio.c_cflag |=  CSTOPB;
        
        newtio.c_iflag &= ~(IXON);
        newtio.c_cc[VTIME]  = 0;
        newtio.c_cc[VMIN] = 0;

        tcflush(m_fd,TCIFLUSH);
        newtio.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
        newtio.c_oflag  &= ~OPOST;

        if((tcsetattr(m_fd,TCSANOW,&newtio))!=0)
        {
            cli::OutputDevice::GetStdErr() << "tcsetattr Error" << cli::endl;
            return false;
        }

        return true;
    }

    bool CCliAgentMcuIoUart::Open()
    {
        m_fd = open(m_IoConfig.strUartPath, O_RDWR );
        
        if(m_fd < 0)
        {
            return false;
        }

        return true;
    }
}