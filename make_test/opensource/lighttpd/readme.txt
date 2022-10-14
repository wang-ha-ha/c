INSTALL
=======
Execute install.sh to copy files into rootfs.


RUN
===
    ov798:/$ cd /html/httpd
    ov798:/html/httpd$ ./start.sh
    [   12.818] random: lighttpd urandom read with 101 bits of entropy available
    ov798:/html/httpd$

Then it should be able to access `https://xxx.xxx.xxx.xxxx/index.html` from a PC web brower on the same network.
xxx.xxx.xxx.xxx represents IP address of OVT IoT SoC.
