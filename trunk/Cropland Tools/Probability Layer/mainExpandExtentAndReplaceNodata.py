import sys
import os
import arcgisscripting

from mainExpandRasterExtent import ExpandRasterExtent
from mainReplaceNodata import ReplaceNodata

if __name__ == '__main__':
    inRaster = sys.argv[1]
    extentRaster = sys.argv[2]
    outRaster = sys.argv[3]
    
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1
    
    interResultRaster = os.getcwd() + "\\" + "interResult"

    ExpandRasterExtent(inRaster, extentRaster, interResultRaster)
    ReplaceNodata(interResultRaster, outRaster)

    gp.delete_management(interResultRaster)
