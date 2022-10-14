#!/bin/sh

count=0
while [ $count -lt 5 ]; do
        sleep 1
        testjpg ${count}.jpg &> /tmp/testjpg.log
        html="$html <a href=\"${count}.jpg\"><img src=\"${count}.jpg\" width=\"30%\" height=\"30%\"></a>"
        let count=count+1
done

cat << EOF
<html>
<h1>OV Linux - JPEG Test Page</h1>
<body>hostname: $(hostname)<br>

$html

</body>
</html>
EOF
