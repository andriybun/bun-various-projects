# -*- coding: utf-8 -*-
"""
Created on Mon Oct 24, 2011

@author: AndriyBun
"""

import sys
import os
import arcgisscripting

def AddMultipleRasters(inRasterList, outRaster):
    
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1
    gp.Extent = "MAXOF"
    
    if len(inRasterList) == 1:
        gp.Copy_management(inRasterList[0], outRaster)
    elif len(inRasterList) == 2:
        gp.Plus_sa(inRasterList[0], inRasterList[1], outRaster)
    else:
        extension = os.path.splitext(os.path.basename(inRasterList[0]))[1]
        tmpRaster = os.path.dirname(outRaster) + "\\" + "tmpRaster" + extension
        gp.AddMessage('adding raster: ' + inRasterList[0])
        gp.Copy_management(inRasterList[0], tmpRaster)
        for raster in inRasterList[1:]:
            gp.AddMessage('adding raster: ' + raster)
            gp.Plus_sa(tmpRaster, raster, outRaster)
            gp.Copy_management(outRaster, tmpRaster)
        gp.AddMessage('deleting temporary data')
        gp.delete_management(tmpRaster)

if __name__ == '__main__':
    inRasterList = []
    for raster in sys.argv[1].split(";"):
        if (raster[0] == '\''):
            raster = raster[1:-1]
        inRasterList.append(raster)

    outRaster = sys.argv[2]
    
    AddMultipleRasters(inRasterList, outRaster)