#!/bin/sh

testmp4 &> /tmp/testmp4.log

cat << EOF
<html>
<h1>OV Linux - MP4 Test Page</h1>
<body>hostname: oa7000<br>

<video controls autoplay muted width="50%" height="50%" src="h264.mp4" type="video/mp4">
Your browser doesn't support the video tag.
</video>

</body>
</html>
EOF
