BUILD_DIR = ./build
DEST_DIR = ./build
DEST_EXE_NAME = yaml_test

all: build
	cd $(BUILD_DIR);make

run:
	cd $(DEST_DIR); ./$(DEST_EXE_NAME)

clean:
	rm $(BUILD_DIR) -rf

build:
	mkdir $(BUILD_DIR);cd $(BUILD_DIR);cmake ..
