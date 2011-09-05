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
    
    extension = os.path.splitext(os.path.basename)[1]
    
    #tmpRaster = os.getcwd() + "\\" + "tmpRaster" + extension

    # TODO: add warning if rasters overlap
    gp.Con_sa(inRaster1, inRaster1, inRaster2, outRaster, "(VALUE >= 0) OR (VALUE < 0)")
    
    #gp.delete_management(tmpRaster)


if __name__ == '__main__':
    inRaster1 = sys.argv[1]
    inRaster2 = sys.argv[2]
    outRaster = sys.argv[3]
    MergeTwoRasters(inRaster1, inRaster2, outRaster)