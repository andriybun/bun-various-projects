# -*- coding: utf-8 -*-
"""
Created on Mon Sep 05 20:37:36 2011

@author: AndriyB0un
"""

import sys
import os
import arcgisscripting

def MergeTwoRasters(inRaster1, inRaster2, outRaster):
    
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1
    gp.Extent = "MAXOF"
    
    extension = os.path.splitext(os.path.basename(inRaster1))[1]
    
    tmpRaster = os.path.dirname(outRaster) + "\\" + "tmpMerge" + extension

    # TODO: add warning if rasters overlap
    # TODO: error if different extensions (check)
    gp.Con_sa(inRaster1, inRaster1, tmpRaster, inRaster2, "(VALUE >= 0) OR (VALUE < 0)")
    gp.Copy_management(tmpRaster, outRaster)
    
    gp.delete_management(tmpRaster)


if __name__ == '__main__':
    inRaster1 = sys.argv[1]
    inRaster2 = sys.argv[2]
    outRaster = sys.argv[3]
    MergeTwoRasters(inRaster1, inRaster2, outRaster)