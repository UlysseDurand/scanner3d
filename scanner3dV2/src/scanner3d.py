from PIL import Image
import os
from shapely.geometry import Polygon, MultiPolygon
from shapely.affinity import rotate
import plotly.graph_objects as go

from src.geom_util import intersection_multipoly, union_multipoly
from src.polygon_utils import extractpoints, mytriangulate, draw_multipolygon, ptinlist, trianglestofaces, findpointid
from src.mymesh import MyMesh
from src.config import Scanner3dConfig, get_config

def load_images(config, folder_path):
    '''
    Image format : 00001.png
    '''
    images = [None] * config.nb_images
    for filename in os.listdir(folder_path):
        if filename.endswith(".png"):
            img_path = os.path.join(folder_path, filename)
            img = Image.open(img_path).convert("L")
            images[int(filename[:5])-1] = img
    return images

def processLayer(config, images, layer: int):
    '''
    For a given value of y, gives the list of the shadows of the layer at that
    height rotated at their corresponding angle
    '''
    longdistance = 3000
    multipolystointer = []
    for i in range(0, len(images), 1):
        # Computes the shadow of the layer i in the image of the shadoow at
        # angle i*180/30 in degrees
        polytounion = []
        points = []
        for x in range(-1, config.width):
            a = False if (x == -1) else images[i].getpixel((x, layer)) <= 120
            b = False if (x >= config.width - 1) else images[i].getpixel((x + 1, layer)) <= 120
            if (not (a) and b):
                points.append((x+1-config.width/2, longdistance))
                points.append((x+1-config.width/2, -longdistance))
            if (a and not (b)):
                points.append((x+1-config.width/2, -longdistance))
                points.append((x+1-config.width/2, longdistance))
                thepoly = rotate(Polygon(points), angle=i * 180 / 30, origin=(0,0))
                polytounion.append(thepoly)
                points = []
        multipolystointer.append(union_multipoly(polytounion))
    return multipolystointer

def getLayers(config, images):
    '''
    From the shadows of a 3D object, returns one slice of the object for
    every height
    '''
    return [intersection_multipoly(processLayer(config, images, i)) for i in range(config.height)]

def mergetwoslices(mesh, poly1, poly2, y, visualize=None):
    polyinter = poly1.intersection(poly2)
    
    # Identify uniquely every point needed and add it to vertices
    thosepoints = []
    for pt in extractpoints(poly1):
        # if pt not in polyinter
        if not(ptinlist(extractpoints(polyinter), pt)):
            thosepoints.append((mesh.addVertex(pt), pt))
    for pt in extractpoints(poly2):
        # if pt not in polyinter
        if not(ptinlist(extractpoints(polyinter), pt)):
            thosepoints.append((mesh.addVertex(pt), pt))
    for pt in extractpoints(polyinter):
        thosepoints.append((mesh.addVertex(pt), pt))

    mesh.setpointsheight(map(lambda pair: pair[0], thosepoints), y)

    if visualize:
        xs, ys = zip(*map(lambda pt:pt[1],thosepoints))
        visualizeRes = [go.Scatter(x=xs, y=ys, mode="markers", name="points")]

    # triangulate the face poly1-poly2 and poly2-poly1
    poly1m2 = poly1.difference(poly2)
    poly2m1 = poly2.difference(poly1)
        
    triangles1m2 = mytriangulate(poly1m2)
    triangles2m1 = mytriangulate(poly2m1)
    
    if visualize:
        traces1m2 = draw_multipolygon(MultiPolygon(triangles1m2), color='red', alpha=0.1)
        traces2m1 = draw_multipolygon(MultiPolygon(triangles2m1), color='black', alpha=0.1)
        visualizeRes += traces1m2 + traces2m1
    else:
        visualizeRes = None

    mesh.faces += trianglestofaces(thosepoints, triangles1m2)
    mesh.faces += trianglestofaces(thosepoints, triangles2m1, reverse=True)

    return thosepoints, visualizeRes

def layerstomesh(config, layers):
    '''Given the layers in the form of multipolygons, returns the reconstructed
    mesh'''
    res = MyMesh()
    previouspoints = []
    for i in range(-1, config.height):
        poly1 = MultiPolygon() if (i == -1) else layers[i]
        poly2 = MultiPolygon() if (i == config.height - 1) else layers[i + 1]
        y = config.height / 2 - i - 0.5

        #build side faces
        thosepoints, _ = mergetwoslices(res, poly1, poly2, y)
        res.addFaceToFace(previouspoints, thosepoints, poly1)
        previouspoints = thosepoints
    return res

def shadowToObj(config, shadows):
    '''
    Converts shadow images to a MyMesh
    '''
    layers = getLayers(config, shadows)
    return layerstomesh(config, layers)

def main(config):
    '''
    Main pipeline
    '''
    print(config.__dict__)


    print("Processing...")
    shadows = load_images(config, config.input_folder)
    mesh = shadowToObj(config, shadows)

    mesh.export(config.output_folder)
    print(f"Done, result saved to {config.output_folder}")


if __name__ == '__main__':
    config = get_config()
    main(config)