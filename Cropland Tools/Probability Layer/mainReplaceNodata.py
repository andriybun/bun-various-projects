# -*- coding: utf-8 -*-
"""
Replace noData values with zeros

Created on Wed May 11 11:03:09 2011

@author: Anrdiy Bun
"""

import sys
import os
import arcgisscripting

#from GeoprocessingInfo import GeoprocessingInfo, GeoprocessingInfo_debug

def ReplaceNodata(inRaster, outRaster):
    #gui = GeoprocessingInfo()
    
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1
    
    tmpRaster = os.getcwd() + "\\" + "tmpRaster"

    gp.IsNull_sa(inRaster, tmpRaster)
    gp.Con_sa(tmpRaster, 0, outRaster, inRaster, "VALUE = 1")
    
    gp.delete_management(tmpRaster)


if __name__ == '__main__':
#    inRaster = sys.argv[1]
#    outRaster = sys.argv[2]
    inRaster = 'd:\path'
    outRaster = 'd:\result'
    ReplaceNodata(inRaster, outRaster)