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

    extension = os.path.splitext(os.path.basename)[1]
    
    tmpRaster = os.getcwd() + "\\" + "tmpRaster" + extension
    tmpRaster2 = os.getcwd() + "\\" + "tmpRaster" + extension

    if len(inRasterList) == 1:
        gp.Copy_management(inRasterList, outRaster)
    elif len(inRasterList) == 2:
        MergeTwoRasters(inRasterList[0], inRasterList[1], outRaster)
    else:
        MergeTwoRasters(inRasterList[0], inRasterList[1], tmpRaster)
        for raster in inRasterList[2:]:
            MergeTwoRasters(tmpRaster, raster, tmpRaster2)
            gp.Copy_management(tmpRaster2, tmpRaster)

    gp.delete_management(tmpRaster)
    gp.delete_management(tmpRaster2)

if __name__ == '__main__':
    inRasterList = sys.argv[1].split(";")

    for i in range(len(inRasterList)):
        if (inRasterList[i][0] == '\''):
            inRasterList[i] = inRasterList[i][1:-1]
    outRaster = sys.argv[2]
    
    MergeMultipleRasters(inRasterList, outRaster)