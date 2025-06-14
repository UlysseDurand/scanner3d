import argparse

class Scanner3dConfig:
    def __init__(self):
        self.input_folder: str
        self.output_path: str
        self.nb_images: int
        self.width: int
        self.height: int

def get_config(*args, **kwargs) -> Scanner3dConfig:
    parser = argparse.ArgumentParser(**kwargs)

    parser.add_argument("-i", "--input_folder", type=str, default="data", help="Path to input folder")
    parser.add_argument("-o", "--output_path", type=str, default="output.obj", help="Path to output file")
    parser.add_argument("-N", "--nb_images", type=int, default=30, help="Number of images to process")
    parser.add_argument("-W", "--width", type=int, default=250, help="Image width")
    parser.add_argument("-H", "--height", type=int, default=150, help="Image height")

    parsed_args = parser.parse_args(args if args else None)

    config = Scanner3dConfig()
    config.input_folder=parsed_args.input_folder
    config.output_folder=parsed_args.output_path
    config.nb_images=parsed_args.nb_images
    config.width=parsed_args.width
    config.height=parsed_args.height
    
    return config