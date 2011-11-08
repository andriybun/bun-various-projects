import math
import os
import arcgisscripting

#===============================================================================
# Common handy functions
#===============================================================================
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
    
#===============================================================================
# ArcGIS cripting functions
#===============================================================================

# Checks if all the rasters in a list have the same cell size
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

# Gets extent of all data (in opposite to NoData) in a raster
def GetDataExtent(inRaster, gp = None):
    if gp is None:
        gp = arcgisscripting.create ()
        gp.OverWriteOutput = 1
    tmpRaster = os.getcwd() + "\\tempgde.img"
    tmpShape = os.getcwd() + "\\tempgde.shp"
    gp.Int_sa(inRaster, tmpRaster)
    gp.RasterToPolygon_conversion(tmpRaster, tmpShape, "NO_SIMPLIFY")
    desc = gp.Describe(tmpShape)
    extent = desc.Extent
    gp.Delete_management(tmpShape)
    gp.Delete_management(tmpRaster)
    return extent

def CreateMaskLayer(inRaster, condition, outMask, gp = None):
    if gp is None:
        gp = arcgisscripting.create ()
        gp.OverWriteOutput = 1
    tmpRaster = os.getcwd() + "\\tempcml.img"
    gp.Con_sa(inRaster, 1, tmpRaster, "#", condition)
    dataExtent = GetDataExtent(tmpRaster, gp)
    gp.Clip_management(tmpRaster, dataExtent, outMask)
    gp.Delete_management(tmpRaster)

def ConClip(inConditionalRaster, inValueRaster, condition, outRaster, gp = None):
    if gp is None:
        gp = arcgisscripting.create ()
        gp.OverWriteOutput = 1
    mask = os.getcwd() + "\\tempcc.img"
    gp.AddMessage("Condition: " + condition)
    CreateMaskLayer(inConditionalRaster, condition, mask, gp)
    gp.Con_sa(mask, inValueRaster, outRaster, "#", "value = 1")
    gp.Delete_management(mask)

if __name__ == "__main__":
    inConditionalLayer = r"m:\Andriy\new_run\1_Cropland_Validation_Inputs\img\countries.img"
    outMask = r"m:\Andriy\new_run\Results_Global\tmp_PC98199\xxx_tunisia_mask.img"
    outLayer = r"m:\Andriy\new_run\Results_Global\tmp_PC98199\xxx_tunisia.img"
    CreateMaskLayer(inConditionalLayer, "COUNTRY = 'Tunisia'", outMask)
    ConClip(inLayer, inLayer, "COUNTRY = 'Tunisia'", outLayer)