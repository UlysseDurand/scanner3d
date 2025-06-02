.PHONY: build run

BUILD_FOLDER=build
IMAGE_NAME=scanner3d
OUTPUT_PATH=output.obj

NB_IMAGES=45
WIDTH=518
HEIGHT=518
VOXEL_NX=200
VOXEL_NY=200
VOXEL_NZ=200

build:
	cd $(BUILD_FOLDER) && cmake .. && cmake --build .

run:
	$(BUILD_FOLDER)/$(IMAGE_NAME) \
	  -i data/ \
	  -o $(OUTPUT_PATH) \
	  -N $(NB_IMAGES) \
	  -W $(WIDTH) \
	  -H $(HEIGHT) \
	  -x $(VOXEL_NX) \
	  -y $(VOXEL_NY) \
	  -z $(VOXEL_NZ)

docker-build:
	docker build -t scanner3dv1 .

docker-run:
	docker run --name scanner3d_container scanner3dv1
	docker cp scanner3d_container:/app/$(OUTPUT_PATH) $(OUTPUT_PATH)
	docker rm scanner3d_container
