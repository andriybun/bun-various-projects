# -*- coding: utf-8 -*-
"""
Expand raster's extent to an extend of another raster

Created on Wed May 11 10:05:33 2011

@author: Andriy Bun
"""

import sys
import os
import arcgisscripting

#from GeoprocessingInfo import GeoprocessingInfo, GeoprocessingInfo_debug

def GetRasterExtent(raster):
    gp = arcgisscripting.create()
    extent = str(gp.GetRasterProperties(extentRaster, 'LEFT')) + ' ' + \
        str(gp.GetRasterProperties(extentRaster, 'BOTTOM')) + ' ' + \
        str(gp.GetRasterProperties(extentRaster, 'RIGHT')) + ' ' + \
        str(gp.GetRasterProperties(extentRaster, 'TOP'))
    return extent

def ExpandRasterExtent(inRaster, extentRaster, outRaster):
    #gui = GeoprocessingInfo()
    
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1
    
    TMPDIR = os.getcwd() + "\\"
    tmpRaster = TMPDIR + "tmpRaster"
    
    #getting raster properties
    typesList = ['DUMMY', 'INTEGER', 'FLOAT']
    dataType = typesList[int(gp.GetRasterProperties(extentRaster, 'VALUETYPE'))]
    #gui.PrintText(GetRasterExtent(extentRaster))
    gp.CreateConstantRaster_sa(tmpRaster, 0, dataType, inRaster, GetRasterExtent(extentRaster))
    
    gp.Extent = "MAXOF"
    gp.Plus_sa(inRaster, tmpRaster, outRaster)
    
    gp.delete_management(tmpRaster)


if __name__ == '__main__':
    inRaster = sys.argv[1]
    extentRaster = sys.argv[2]
    outRaster = sys.argv[3]
    ExpandRasterExtent(inRaster, extentRaster, outRaster)