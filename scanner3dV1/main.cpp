#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <cmath>

#include <Eigen/Dense>
using namespace Eigen;

using namespace std;
namespace fs = filesystem;

#define MC_IMPLEM_ENABLE
// https://github.com/aparis69/MarchingCubeCpp
#include "MC.h"

const int HEIGHT = 518;
const int WIDTH = 518;
const int NB_IMAGES = 45;
const string INPUT_FOLDER = "input"; 
const string OUTPUT_FILE = "out.obj";

const int VOXEL_NX = 200;
const int VOXEL_NY = 200;
const int VOXEL_NZ = 200;

typedef bool* Image; // pixel (x,y) at img[y * WIDTH + x]
Image initImage() {return new bool[WIDTH * HEIGHT]();}
bool getPxValue(Image img, int x, int y) {return img[y * WIDTH + x];}
void setPxValue(Image img, int x, int y, bool v) {img[y * WIDTH + x] = v;}

typedef array<Image, NB_IMAGES> Acquisition;

typedef bool* VoxelGrid; // voxel (x,y,z) at grid[x * HEIGHT * WIDTH + y * WIDTH + z]
VoxelGrid initVoxelGrid() {return new bool[VOXEL_NX * VOXEL_NY * VOXEL_NZ]();}
bool getValue(VoxelGrid grid, int x, int y, int z) {return grid[z * VOXEL_NY * VOXEL_NX + y * VOXEL_NX + x];}
void setValue(VoxelGrid grid, int x, int y, int z, bool v) {grid[z * VOXEL_NY * VOXEL_NX + y * VOXEL_NX + x] = v;}

Image cvImageToImage(const cv::Mat& img) {
    Image result = initImage();
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
            setPxValue(result, x, y, pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0);
        }
    }
    return result;
}

Acquisition processImagesInFolder(const string& folderPath) {
    Acquisition result = {};
    cout << folderPath << endl;
    if (!fs::exists(folderPath)) {
        std::cerr << "Folder does not exist!\n";
        return result;
    }
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".png") {
            string filename = entry.path().filename();
            int imgnb = stoi(filename.substr(0, filename.find('.'))); 

            cv::Mat img = cv::imread(entry.path().string(), cv::IMREAD_COLOR);
            if (img.empty()) {
                cerr << "Failed to read: " << entry.path() << endl;
                continue;
            }
            if (img.rows != HEIGHT || img.cols != WIDTH) {
                cerr << "Skipping " << filename << " due to incorrect dimensions.\n";
                continue;
            }
            result[imgnb] = cvImageToImage(img);
        }
    }
    return result;
}

// projects point on plane with normal as normal vector and exppress its
// coordinates in the (u,v) basis
Vector2f project3dOn2d(Vector3f point, Vector3f u, Vector3f v, Vector3f normal) {
   float distance = point.dot(normal); 
   Vector3f projectedOnPlane = point - distance * normal;
   Vector2f result(projectedOnPlane.dot(u), projectedOnPlane.dot(v));
   return result;
}

// affine transformation from [-VOXEL_NX/2,VOXEL_NX/2]*[-VOXEL_NY/2, VOXEL_NY/2]
// to [0, WIDTH]*[0,HEIGHT] 
Vector2f toCoordImage(Vector2f coords) {
    return Vector2f(coords(0)*WIDTH/VOXEL_NX + WIDTH / 2, coords(1)*HEIGHT/VOXEL_NY + HEIGHT / 2);
}

// Process each voxel to see wether or not it is in all the shadows (with
// respective angle) from acq
VoxelGrid filterFromAcquisition(Acquisition acq) {
    VoxelGrid grid = initVoxelGrid();
    for (int x=0; x<VOXEL_NX; ++x) {
        for (int y=0; y<VOXEL_NY; ++y) {
            for (int z=0; z<VOXEL_NZ; ++z) {
                // setValue(grid, x, y, z, (x+y+z)%40 == 0);
                bool value = true;
                // point range : [-VOXEL_NX/2; VOXEL_NX/2]*[-VOXEL_NY/2; VOXEL_NY/2]*[-VOXEL_NZ/2; VOXEL_NZ/2]
                Vector3f point(x - VOXEL_NX / 2, y - VOXEL_NY / 2, z - VOXEL_NZ / 2);
                for (int i=0; i<NB_IMAGES && value; ++i) {
                    float theta = i * 2 * M_PI / NB_IMAGES;
                    float costheta = cos(theta);
                    float sintheta = sin(theta);
                    Vector3f planeU(costheta, sintheta, 0.0f);
                    Vector3f planeV(0.0f, 0.0f, 1.0f);
                    Vector3f planeNormal(-sintheta, costheta, 0.0f);
                    Vector2f projectedPoint = project3dOn2d(point, planeU, planeV, planeNormal);
                    Vector2f imageCoordinates = toCoordImage(projectedPoint);
                    int imgx = (int)imageCoordinates(0);
                    int imgy = (int)imageCoordinates(1);
                    value = value & getPxValue(acq[i],imgx, imgy);
                }
                setValue(grid, x, y, z, value);
            }
        }
    }
    return grid;
}

void marchingCubes(VoxelGrid grid) {
    string result = "";

    MC::MC_FLOAT* field = new MC::MC_FLOAT[WIDTH*HEIGHT*WIDTH];
    for (int x = 0; x < VOXEL_NX; ++x) {
        for (int y = 0; y < VOXEL_NY; ++y) {
            for (int z = 0; z < VOXEL_NZ; ++z) {
                float value = getValue(grid, x, y, z) ? 1.0f : -1.0f;
                field[z * VOXEL_NY * VOXEL_NX + y * VOXEL_NX + x] = value;
            }
        }
    }
    MC::mcMesh mesh;
    MC::marching_cube(field, VOXEL_NX, VOXEL_NY, VOXEL_NZ, mesh);

    // Writing the obj file
    ofstream out;
    out.open(OUTPUT_FILE);
    if (!out.is_open())
        return;
    out << "g Obj" << endl;
	for (size_t i = 0; i < mesh.vertices.size(); i++)
        out << "v " << mesh.vertices.at(i).x << " " << mesh.vertices.at(i).y << " " << mesh.vertices.at(i).z << '\n';
    for (size_t i = 0; i < mesh.vertices.size(); i++)
        out << "vn " << mesh.normals.at(i).x << " " << mesh.normals.at(i).y << " " << mesh.normals.at(i).z << '\n';
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        out << "f " << mesh.indices.at(i) + 1 << "//" << mesh.indices.at(i) + 1
            << " " << mesh.indices.at(i + 1) + 1 << "//" << mesh.indices.at(i + 1) + 1
            << " " << mesh.indices.at(i + 2) + 1 << "//" << mesh.indices.at(i + 2) + 1
            << '\n';
    }
}

int main() {

    cout << "OpenCV version: " << CV_VERSION << endl;

	Acquisition images = processImagesInFolder(INPUT_FOLDER);

    cout << "Images read" << endl;

    VoxelGrid grid = filterFromAcquisition(images);

    cout << "Applying Marching Cubes" << endl;

    marchingCubes(grid);

    cout << "File "<< OUTPUT_FILE <<" written" << endl;

    return 0;
}

