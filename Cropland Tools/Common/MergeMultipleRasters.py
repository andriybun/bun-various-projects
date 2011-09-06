# -*- coding: utf-8 -*-
"""
Created on Mon Sep 05 21:44:05 2011

@author: AndriyBun
"""

import sys
import os
import arcgisscripting
from MergeTwoRasters import MergeTwoRasters

def MergeMultipleRasters(inRasterList, outRaster):
    
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1
    gp.Extent = "MAXOF"
    
    extension = os.path.splitext(os.path.basename(inRasterList[0]))[1]
    
    tmpRaster = os.path.dirname(outRaster) + "\\" + "tmpRaster" + extension

    if len(inRasterList) == 1:
        gp.Int_sa(inRasterList[0], outRaster)
    elif len(inRasterList) == 2:
        MergeTwoRasters(inRasterList[0], inRasterList[1], outRaster)
    else:
        gp.AddMessage('adding raster: ' + inRasterList[0])
        gp.Int_sa(inRasterList[0], outRaster)
        for raster in inRasterList[1:]:
            gp.AddMessage('adding raster: ' + raster)
            gp.Int_sa(raster, tmpRaster)
            MergeTwoRasters(outRaster, tmpRaster, outRaster)
#        gp.AddMessage('deleting temporary data')
#        gp.delete_management(tmpRaster)

if __name__ == '__main__':
    inRasterList = []
    for raster in sys.argv[1].split(";"):
        if (raster[0] == '\''):
            raster = raster[1:-1]
        inRasterList.append(raster)
    outRaster = sys.argv[2]
    
    MergeMultipleRasters(inRasterList, outRaster)