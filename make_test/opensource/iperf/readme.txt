# iperf2.0.5 - sample log

    oa7000:/ftp$ ./iperf -s
    ------------------------------------------------------------
    Server listening on TCP port 5001
    TCP window size: 85.3 KByte (default)
    ------------------------------------------------------------
    [  4] local 192.168.1.64 port 5001 connected with 192.168.1.4 port 65497
    [ ID] Interval       Transfer     Bandwidth
    [  4]  0.0-10.1 sec  30.4 MBytes  25.3 Mbits/sec


    C:\iperf-2.0.8b-win64>iperf -c 192.168.1.64
    ------------------------------------------------------------
    Client connecting to 192.168.1.64, TCP port 5001
    TCP window size:  208 KByte (default)
    ------------------------------------------------------------
    [  3] local 192.168.1.4 port 65497 connected with 192.168.1.64 port 5001
    [ ID] Interval       Transfer     Bandwidth
    [  3]  0.0-10.0 sec  30.4 MBytes  25.4 Mbits/sec

END
