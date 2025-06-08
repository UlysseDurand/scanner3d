import numpy as np
from shapely.geometry import Polygon, MultiPolygon
import plotly.graph_objects as go

from src.polygon_utils import findpointid

class MyMesh:
    def __init__(self):
        self.vertices = []
        self.faces = []
        self.vertexID = 0

    def addVertex(self, pt):
        self.vertices.append(pt)
        self.vertexID += 1
        return self.vertexID - 1

    def addRectangle(self, aID, bID, cID, dID):
        self.faces += [(aID, bID, cID), (cID, dID, aID)]

    def addFaceToFace(self, ids1, ids2, multipol, reverse=False):
        if (isinstance(multipol, Polygon)):
            n = len(multipol.exterior.coords)
            for i in range(n):
                pt1 = multipol.exterior.coords[i]
                pt2 = multipol.exterior.coords[(i+1)%n]
                a = findpointid(ids1, pt2)
                b = findpointid(ids1, pt1)
                c = findpointid(ids2, pt1)
                d = findpointid(ids2, pt2)
                if (reverse):
                    self.addRectangle(d, c, b, a)
                else:
                    self.addRectangle(a, b, c, d)
            for interior in multipol.interiors:
                self.addFaceToFace(ids1, ids2, interior, reverse=True)
        elif (isinstance(multipol, MultiPolygon)):
            for pol in multipol.geoms:
                self.addFaceToFace(ids1, ids2, pol)

    def setpointsheight(self, pointidx, height):
        for ptid in pointidx:
            v = self.vertices[ptid]
            self.vertices[ptid] = (v[0], height, v[1])

    def addFace(self, f):
        self.faces.append(f)

    def plot(self):
        
        x, y, z = np.array(self.vertices).T
        i, j, k = np.array(self.faces).T

        fig = go.Figure(data=[go.Mesh3d(x=x, y=y, z=z, i=i, j=j, k=k)])
        fig.show()

    def export(self, filename):
        with open(filename, 'w') as f:
            for v in self.vertices:
                f.write(f"v {v[0]} {v[1]} {v[2]}\n")
            for (a,b,c) in self.faces:
                f.write(f"f {a+1} {b+1} {c+1}\n")