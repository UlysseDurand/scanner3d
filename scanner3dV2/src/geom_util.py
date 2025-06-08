from shapely.geometry import MultiPolygon, GeometryCollection, Polygon, LineString, Point

# Boolean operations with MultiPolygons

def union_multipoly(listmpoly):
    if len(listmpoly) == 0:
        return MultiPolygon()
    else:
        res = listmpoly[0]
        for mpoly in listmpoly:
            res = res.union(mpoly)
        return res

def intersection_multipoly(listmpoly):
    if len(listmpoly) == 0:
        raise ValueError("Empty intersection")
    else:
        res = listmpoly[0]
        for mpoly in listmpoly:
            res = res.intersection(mpoly)
        if isinstance(res, Polygon):
            return MultiPolygon([res])
        elif isinstance(res, MultiPolygon):
            return res
        else:
            print(res)
            return res