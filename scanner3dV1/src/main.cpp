#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <cmath>

#include <opencv2/opencv.hpp>

#include <Eigen/Dense>
using namespace Eigen;

#include <CLI/CLI.hpp>

using namespace std;
namespace fs = filesystem;

#define MC_IMPLEM_ENABLE
// https://github.com/aparis69/MarchingCubeCpp
#include "MC.h"

/**
 * @brief Configuration parameters for the 3D scanner
 *
 * Contains all the necessary parameters to configure the scanning process
 * including image dimensions, voxel grid size, and file paths.
 */
struct Config
{
    int WIDTH;
    int HEIGHT;
    int NB_IMAGES;
    int VOXEL_NX;
    int VOXEL_NY;
    int VOXEL_NZ;
    string INPUT_FOLDER;
    string OUTPUT_FILE;
    bool verbose;
};

/**
 * @brief Image datatype representing a 2D binary image
 *
 * Pixels are stored in row-major order: pixel (x,y) at img[y * WIDTH + x]
 * Uses unique_ptr for automatic memory management
 */
typedef unique_ptr<bool[]> Image;
Image initImage(Config config)
{
    bool *img = new (std::nothrow) bool[config.WIDTH * config.HEIGHT]();
    if (!img)
    {
        throw std::runtime_error("Failed to allocate image memory");
    }
    return img;
}
bool getPxValue(Config config, Image img, int x, int y)
{
    int WIDTH = config.WIDTH;
    return img[y * WIDTH + x];
}
void setPxValue(Config config, Image img, int x, int y, bool v)
{
    int WIDTH = config.WIDTH;
    img[y * WIDTH + x] = v;
}

/**
 * @brief Collection of images representing different views
 *
 * A vector of Image pointers representing the acquired shadow images
 * from different angles around the object.
 */
typedef vector<Image> Acquisition;
Acquisition initAcquisition(Config config)
{
    Acquisition res(config.NB_IMAGES);
    return res;
}

/**
 * @brief 3D voxel grid datatype
 *
 * Represents a 3D volume where each voxel is either occupied (true) or empty (false).
 * Voxels are stored in z-major order: voxel (x,y,z) at grid[z * VOXEL_NY * VOXEL_NX + y * VOXEL_NX + x]
 * Uses unique_ptr for automatic memory management
 */
typedef unique_ptr<bool[]> VoxelGrid;
VoxelGrid initVoxelGrid(Config config)
{
    int VOXEL_NX = config.VOXEL_NX;
    int VOXEL_NY = config.VOXEL_NY;
    int VOXEL_NZ = config.VOXEL_NZ;
    bool *grid = new (std::nothrow) bool[VOXEL_NX * VOXEL_NY * VOXEL_NZ]();
    if (!grid)
    {
        throw std::runtime_error("Failed to allocate voxel grid memory");
    }
    return grid;
}
bool getValue(
    Config config,
    VoxelGrid grid, int x, int y, int z)
{
    int VOXEL_NX = config.VOXEL_NX;
    int VOXEL_NY = config.VOXEL_NY;
    int VOXEL_NZ = config.VOXEL_NZ;
    return grid[z * VOXEL_NY * VOXEL_NX + y * VOXEL_NX + x];
}
void setValue(
    Config config,
    VoxelGrid grid, int x, int y, int z, bool v)
{
    int VOXEL_NX = config.VOXEL_NX;
    int VOXEL_NY = config.VOXEL_NY;
    int VOXEL_NZ = config.VOXEL_NZ;
    grid[z * VOXEL_NY * VOXEL_NX + y * VOXEL_NX + x] = v;
}

/**
 * @brief Converts an OpenCV image to our binary Image format
 *
 * @param config Scanner configuration containing image dimensions
 * @param img Input OpenCV image (expected to be 3-channel BGR)
 * @return Image Converted binary image (true for black pixels, false for others)
 * @throws std::runtime_error if memory allocation fails
 */
Image cvImageToImage(Config config, const cv::Mat &img)
{
    int WIDTH = config.WIDTH;
    int HEIGHT = config.HEIGHT;
    Image result = initImage(config);
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
            setPxValue(config, result, x, y, pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0);
        }
    }
    return result;
}

/*
GEOMETRY UTILS
*/

/**
 * @brief Projects a 3D point onto a 2D plane
 *
 * @param point 3D point to project
 * @param u First basis vector of the plane
 * @param v Second basis vector of the plane
 * @param normal Plane normal vector
 * @return Vector2f 2D coordinates in the (u,v) basis
 */
Vector2f project3dOn2d(Vector3f point, Vector3f u, Vector3f v, Vector3f normal)
{
    float distance = point.dot(normal);
    Vector3f projectedOnPlane = point - distance * normal;
    Vector2f result(projectedOnPlane.dot(u), projectedOnPlane.dot(v));
    return result;
}

/**
 * @brief Transforms coordinates from voxel space to image space
 *
 * Performs an affine transformation from [-VOXEL_NX/2,VOXEL_NX/2]*[-VOXEL_NY/2, VOXEL_NY/2]
 * to [0, WIDTH]*[0,HEIGHT] image coordinates.
 *
 * @param WIDTH Output image width
 * @param HEIGHT Output image height
 * @param VOXEL_NX Voxel grid X dimension
 * @param VOXEL_NY Voxel grid Y dimension
 * @param coords Input coordinates in voxel space
 * @return Vector2f Transformed coordinates in image space
 */
Vector2f toCoordImage(
    int WIDTH, int HEIGHT, int VOXEL_NX, int VOXEL_NY,
    Vector2f coords)
{
    return Vector2f(coords(0) * WIDTH / VOXEL_NX + WIDTH / 2, coords(1) * HEIGHT / VOXEL_NY + HEIGHT / 2);
}

/*
PIPELINE FUNCTIONS
*/

Acquisition loadImagesInFolder(
    Config config,
    const string &folderPath)
{
    int WIDTH = config.WIDTH;
    int HEIGHT = config.HEIGHT;
    int NB_IMAGES = config.NB_IMAGES;
    bool verbose = config.verbose;

    if (verbose)
    {
        cout << endl
             << "Loading images" << endl;
    }
    Acquisition result = initAcquisition(config);
    if (!fs::exists(folderPath))
    {
        std::cerr << "Folder " << folderPath << " does not exist!\n";
        return result;
    }
    int count = 0;
    for (const auto &entry : fs::directory_iterator(folderPath))
    {
        if (entry.path().extension() == ".png")
        {
            string filename = entry.path().filename();
            int imgnb = stoi(filename.substr(0, filename.find('.')));

            cv::Mat img;
            try
            {
                img = cv::imread(entry.path().string(), cv::IMREAD_COLOR);
                if (img.empty())
                {
                    throw std::runtime_error("Empty image");
                }
                if (img.rows != HEIGHT || img.cols != WIDTH)
                {
                    throw std::runtime_error("Incorrect dimensions");
                }
            }
            catch (const std::exception &e)
            {
                cerr << "Error loading " << entry.path() << ": " << e.what() << endl;
                continue;
            }
            if (img.rows != HEIGHT || img.cols != WIDTH)
            {
                cerr << "Skipping " << filename << " due to incorrect dimensions.\n";
                continue;
            }
            count++;
            result[imgnb] = cvImageToImage(config, img);
            if (verbose)
            {
                cout << "\rProgress: " << count << "/" << NB_IMAGES << flush;
            }
        }
    }
    if (verbose)
    {
        cout << endl;
    }
    return result;
}

// Process each voxel to see wether or not it is in all the shadows (with
// respective angle) from acq
VoxelGrid filterFromAcquisition(
    Config config,
    Acquisition acq)
{
    int VOXEL_NX = config.VOXEL_NX;
    int VOXEL_NY = config.VOXEL_NY;
    int VOXEL_NZ = config.VOXEL_NZ;
    int WIDTH = config.WIDTH;
    int HEIGHT = config.HEIGHT;
    int NB_IMAGES = config.NB_IMAGES;
    bool verbose = config.verbose;
    if (verbose)
    {
        cout << endl
             << "Filtering voxels" << endl;
        cout << "Progress: [";
        for (int i = 0; i < 50; i++)
            cout << " ";
        cout << "] 0%";
    }
    VoxelGrid grid = initVoxelGrid(config);
    int lastProgress = -1;
    for (int x = 0; x < VOXEL_NX; ++x)
    {
        for (int y = 0; y < VOXEL_NY; ++y)
        {
            for (int z = 0; z < VOXEL_NZ; ++z)
            {
                bool value = true;

                // point range : [-VOXEL_NX/2; VOXEL_NX/2]*[-VOXEL_NY/2; VOXEL_NY/2]*[-VOXEL_NZ/2; VOXEL_NZ/2]
                Vector3f point(x - VOXEL_NX / 2, y - VOXEL_NY / 2, z - VOXEL_NZ / 2);

                for (int i = 0; i < NB_IMAGES && value; ++i)
                {
                    float theta = i * 2 * M_PI / NB_IMAGES;
                    float costheta = cos(theta);
                    float sintheta = sin(theta);
                    Vector3f planeU(costheta, sintheta, 0.0f);
                    Vector3f planeV(0.0f, 0.0f, 1.0f);
                    Vector3f planeNormal(-sintheta, costheta, 0.0f);
                    Vector2f projectedPoint = project3dOn2d(point, planeU, planeV, planeNormal);
                    Vector2f imageCoordinates = toCoordImage(WIDTH, HEIGHT, VOXEL_NX, VOXEL_NY, projectedPoint);
                    int imgx = (int)imageCoordinates(0);
                    int imgy = (int)imageCoordinates(1);
                    value = value & getPxValue(config, acq[i], imgx, imgy);
                }
                setValue(config, grid, x, y, z, value);
            }
        }
        if (verbose)
        {
            int progress = (100 * (x + 1)) / VOXEL_NX;
            if (progress != lastProgress)
            {
                lastProgress = progress;
                cout << "\rProgress: [";
                int pos = progress / 2;
                for (int i = 0; i < 50; ++i)
                {
                    if (i < pos)
                        cout << "=";
                    else if (i == pos)
                        cout << ">";
                    else
                        cout << " ";
                }
                cout << "] " << progress << "%" << flush;
            }
        }
    }
    if (verbose)
    {
        cout << endl;
    }
    return grid;
}

void marchingCubes(
    Config config,
    VoxelGrid grid)
{
    int VOXEL_NX = config.VOXEL_NX;
    int VOXEL_NY = config.VOXEL_NY;
    int VOXEL_NZ = config.VOXEL_NZ;
    int WIDTH = config.WIDTH;
    int HEIGHT = config.HEIGHT;
    int NB_IMAGES = config.NB_IMAGES;
    bool verbose = config.verbose;
    string OUTPUT_FILE = config.OUTPUT_FILE;

    string result = "";

    MC::MC_FLOAT *field = new MC::MC_FLOAT[WIDTH * HEIGHT * WIDTH];
    for (int x = 0; x < VOXEL_NX; ++x)
    {
        for (int y = 0; y < VOXEL_NY; ++y)
        {
            for (int z = 0; z < VOXEL_NZ; ++z)
            {
                float value = getValue(config, grid, x, y, z) ? 1.0f : -1.0f;
                field[z * VOXEL_NY * VOXEL_NX + y * VOXEL_NX + x] = value;
            }
        }
    }
    MC::mcMesh mesh;
    MC::marching_cube(field, VOXEL_NX, VOXEL_NY, VOXEL_NZ, mesh);
    delete[] field; // Clean up field memory

    // Writing the obj file
    ofstream out;
    try
    {
        out.open(OUTPUT_FILE);
        if (!out.is_open())
        {
            throw std::runtime_error("Could not open output file");
        }
    }
    catch (const std::exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return;
    }
    out << "g Obj" << endl;
    for (size_t i = 0; i < mesh.vertices.size(); i++)
        out << "v " << mesh.vertices.at(i).x << " " << mesh.vertices.at(i).y << " " << mesh.vertices.at(i).z << '\n';
    for (size_t i = 0; i < mesh.vertices.size(); i++)
        out << "vn " << mesh.normals.at(i).x << " " << mesh.normals.at(i).y << " " << mesh.normals.at(i).z << '\n';
    for (size_t i = 0; i < mesh.indices.size(); i += 3)
    {
        out << "f " << mesh.indices.at(i) + 1 << "//" << mesh.indices.at(i) + 1
            << " " << mesh.indices.at(i + 1) + 1 << "//" << mesh.indices.at(i + 1) + 1
            << " " << mesh.indices.at(i + 2) + 1 << "//" << mesh.indices.at(i + 2) + 1
            << '\n';
    }
    if (verbose)
    {
        cout << endl
             << "File " << OUTPUT_FILE << " written" << endl;
    }
}

int main(int argc, char **argv)
{
    Config config;
    bool verbose = false;

    CLI::App app{"3D Scanner - Reconstructs a 3D object in .obj format from horizontal views of the orthogonal shadows"};
    app.add_option("-i, --input", config.INPUT_FOLDER, "Input folder path (containing shadows of an object)")->required();
    app.add_option("-o, --output", config.OUTPUT_FILE, "Output .obj file path")->required();
    app.add_option("-N, --nb_images", config.NB_IMAGES, "Number of input images")->required();
    app.add_option("-W, --width", config.WIDTH, "Width of the input images")->required();
    app.add_option("-H, --height", config.HEIGHT, "Height of the images")->required();
    app.add_option("-x, --voxel_Nx", config.VOXEL_NX, "Number of voxel along the X dimension for output .obj")->required();
    app.add_option("-y, --voxel_Ny", config.VOXEL_NY, "Number of voxel along the Y dimension for output .obj")->required();
    app.add_option("-z, --voxel_Nz", config.VOXEL_NZ, "Number of voxel along the Z dimension for output .obj")->required();
    app.add_flag("-v,--verbose", config.verbose, "Enable verbose output");
    CLI11_PARSE(app, argc, argv);

    Acquisition images = loadImagesInFolder(config, config.INPUT_FOLDER);

    VoxelGrid grid = filterFromAcquisition(config, images);

    marchingCubes(config, grid);

    // Memory is automatically managed by unique_ptr

    return 0;
}
