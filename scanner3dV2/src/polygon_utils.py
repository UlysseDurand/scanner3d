#!/usr/bin/env python
# coding: utf-8

# In[1]:


get_ipython().run_line_magic('matplotlib', 'ipympl')

from matplotlib import pyplot as plt
from shapely.geometry import MultiPolygon, GeometryCollection, Polygon, LineString, Point, LinearRing
from shapely.ops import triangulate
import plotly.graph_objects as go


# In[2]:


def rgba_str(color, alpha):
        # Simple color name to rgba helper for common colors
        colors = {
            'blue': f'rgba(0,0,255,{alpha})',
            'red': f'rgba(255,0,0,{alpha})',
            'green': f'rgba(0,255,0,{alpha})',
            'black': f'rgba(0,0,0,{alpha})',
            'white': f'rgba(255,255,255,1)'
        }
        return colors.get(color, f'rgba(0,0,255,{alpha})')  # default blue


# In[ ]:


def draw_multipolygon(multipolygon, color="blue", alpha=0.2, showline=False):
    '''Draws a multiploygon (union of polygons) onto ax, it returns a list of
    plotly traces'''
    if isinstance(multipolygon, Polygon):
        multipolygon = MultiPolygon([multipolygon])
    
    traces = []
    
    for polygon in multipolygon.geoms:
        # Exterior
        x, y = polygon.exterior.xy
        x, y = list(x), list(y)
        for interior in polygon.interiors:
            xi, yi = interior.xy
            x += list(xi)
            y += list(yi)

        traces.append(go.Scatter(
            x=x,
            y=y,
            fill='toself',
            fillcolor=rgba_str(color, alpha),
            line=dict(color='rgba(0,0,0,0)' if not(showline) else color),
            mode='lines',
            showlegend=False,
            visible=True
        ))
    return traces

def tracesToFig(traces, title=None):
    res = go.Figure(data=traces)
    res.update_layout(
        xaxis=dict(scaleanchor="y"),
        xaxis_title="x",
        yaxis_title="y",
        title=title
    )
    return res


# In[4]:


def extractpoints(x):
    '''Outputs all the points that are in the shape'''
    if isinstance(x, MultiPolygon) or isinstance(x, GeometryCollection):
        res = []
        for poly in x.geoms:
            res += extractpoints(poly)
        return res
    elif isinstance(x, Polygon):
        res = list(x.exterior.coords)
        for interior in x.interiors:
            res += extractpoints(interior)
        return res
        
    elif isinstance(x, LineString) or isinstance(x, Point) or isinstance(x, LinearRing):
        return list(x.coords)
    else:
        return []


# In[5]:


def barycenter(tri):
    a = tri.exterior.coords[0]
    b = tri.exterior.coords[1]
    c = tri.exterior.coords[2]
    return Point((a[0]+b[0]+c[0])/3, (a[1]+b[1]+c[1])/3)

def mytriangulatepoly(poly):
    '''Triangulates any convex polygon'''
    res = list(filter(lambda tri: poly.contains(barycenter(tri)), triangulate(poly)))
    return res

def mytriangulate(multipoly):
    '''Triangulates any convex polygon or multipolygon of convex polygons'''
    if isinstance(multipoly, MultiPolygon):
        res = []
        for poly in multipoly.geoms:
            res += mytriangulatepoly(poly)
        return res
    return mytriangulatepoly(multipoly)


# In[6]:


if __name__ == "__main__":
    multipolyexample1 = MultiPolygon([Polygon(
        [(0, 0), (0, 1), (0.5, 0.5), (1, 1), (1, 0)], 
        [[(0.4, 0.2), (0.6, 0.2), (0.6, 0.4)]]
    ), Polygon([(-1, -1), (-1.1, -1), (-1.1, -1.1), (-1, -1.1)])])
    traces = draw_multipolygon(multipolyexample1)
    lafig = tracesToFig(traces)
    xs, ys = zip(*extractpoints(multipolyexample1))
    lafig.add_trace(go.Scatter(x=xs, y=ys, mode='markers', name="points"))
    lafig.update_layout(
        xaxis=dict(scaleanchor="y"),
        xaxis_title="x",
        yaxis_title="y",
        title="Example for draw_multipolygon and extractpoints"
    )
    lafig.show()


# In[7]:


if __name__ == "__main__":
    triexample1 = mytriangulate(multipolyexample1)
    print(MultiPolygon(triexample1))
    
    traces1 = draw_multipolygon(multipolyexample1, color='red', alpha=0.2)
    traces2 = draw_multipolygon(MultiPolygon(triexample1), alpha=0.2, showline=True)
    tracesToFig(traces1+traces2).show()


# In[8]:


def distancesq(a, b):
    return (a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2

def ptequalpt(a, b):
    return distancesq(a, b) < 0.000001
    
def findpointid(listofcouplepoints, pt, makeerror = True, visualize = False):
    theid = - 1
    for (a,b) in listofcouplepoints:
        if ptequalpt(pt, b):
            theid = a
    if (makeerror and theid == -1):
        if visualize:
            plt.plot(pt, 'b+', markersize=15)
            plt.draw()
            
        raise ValueError("point "+str(pt)+" not in listofcouplepoints")
    return theid

def ptinlist(listpoints, pt):
    for pt2 in listpoints:
        if ptequalpt(pt, pt2):
            return True
    return False

def trianglestofaces(biglist, triangles, reverse=False, visualize=False):
    res = []
    for tri in triangles:
        pts = extractpoints(tri)
        id1 = findpointid(biglist,pts[0], False, visualize)
        id2 = findpointid(biglist, pts[1], False, visualize)
        id3 = findpointid(biglist, pts[2], False, visualize)
        if reverse:
            res.append((id3, id2, id1))
        else:
            res.append((id1, id2, id3))
    return res

if __name__ == "__main__":
    listcoupleidpoint = [(0, (0, 0)), (1, (0, 1)), (2, (1, 1)), (3, (1, 0))]
    
    print(findpointid(listcoupleidpoint, (1, 1)))
    print(trianglestofaces(listcoupleidpoint, [Polygon([(0, 0), (1, 0), (1, 1)]), Polygon([(1, 1), (0, 1), (0, 0)])]))


# In[9]:


if __name__ == '__main__':
    get_ipython().system('jupyter nbconvert --to script polygon_utils.ipynb --output-dir=../src/')

