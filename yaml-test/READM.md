1、编译安装yaml
	```bash
	git clone https://github.com/jbeder/yaml-cpp
	cd yaml-cpp
	mkdir build;cd build
	cmake -YAML_BUILD_SHARED_LIBS=on ..
	make
	sudo make install
	```
2、单独编译app
	```bash
	g++  -o tt main.cpp -lyaml-cpp  -std=c++11
	```

3、编写测试yaml的小程序，简单封装一下，但是set还没有封装好现在的思路还不行
