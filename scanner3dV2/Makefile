.PHONY: build run

SRC_FOLDER=src
IMAGE_NAME=scanner3d
OUTPUT_PATH=output.obj

NB_IMAGES=30
WIDTH=250
HEIGHT=150

run:
	python -m $(SRC_FOLDER).$(IMAGE_NAME) \
	  -i data/ \
	  -o $(OUTPUT_PATH) \
	  -N $(NB_IMAGES) \
	  -W $(WIDTH) \
	  -H $(HEIGHT)

docker-build:
	docker build -t scanner3dv2 .

docker-run:
	docker run --name scanner3dv2_container scanner3dv2
	docker cp scanner3dv2_container:/app/$(OUTPUT_PATH) $(OUTPUT_PATH)
	docker remove scanner3dv2_container