import math
import arcgisscripting

def MakeListString(inList):
    listString = ""
    for name in inList:
        listString += "\'" + name + "\';"
    listString = listString[0:-1]
    return listString
    
def CompareEq(val1, val2, precision):
    return (math.fabs(val1 - val2) < precision)
    
def CompareNe(val1, val2, precision):
    return not(CompareEq(val1, val2, precision))
    
def CompareCellSize(rasterList):
    EPSILON = 10e-3
    gp = arcgisscripting.create()
    xSize0 = gp.GetRasterProperties(rasterList[0], 'CELLSIZEX')
    ySize0 = gp.GetRasterProperties(rasterList[0], 'CELLSIZEY')
    for raster in rasterList[1:]:
        xSize = gp.GetRasterProperties(raster, 'CELLSIZEX')
        ySize = gp.GetRasterProperties(raster, 'CELLSIZEY')
        if CompareNe(xSize0, xSize, EPSILON):
            return False
        if CompareNe(ySize0, ySize, EPSILON):
            return False
    return True
    
    
