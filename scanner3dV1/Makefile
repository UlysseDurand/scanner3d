.PHONY: build run

BUILD_FOLDER=build
IMAGE_NAME=scanner3d

build:
	cd $(BUILD_FOLDER) && cmake .. && cmake --build .

run:
	$(BUILD_FOLDER)/$(IMAGE_NAME) \
	  -i data/ \
	  -o output.obj \
	  -N 45 \
	  -H 518 \
	  -W 518 \
	  -x 200 \
	  -y 200 \
	  -z 200


