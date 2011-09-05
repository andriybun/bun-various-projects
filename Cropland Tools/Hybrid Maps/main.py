# -*- coding: utf-8 -*-
"""
Created on Fri May 13 13:39:39 2011

@author: Andriy Bun
"""

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '/../Common'
sys.path.append(commonDir)

from iterableStruct import iterableStruct
from GeoprocessingInfo import GeoprocessingInfo, GeoprocessingInfo_debug

from ProduceHybridMap import ProduceHybridMap

if __name__ == '__main__':

#    gui = GeoprocessingInfo()
    gui = GeoprocessingInfo_debug()
    
    #paths.input.landMaps = sys.argv[3].split(";")
    #for i in range(len(paths.input.landMaps)):
    #    paths.input.landMaps[i] = paths.input.landMaps[i][1:-1]
    
    paths = iterableStruct()
    
#    paths.DATA_DIR = 'C:/ProgramFiles/Cropland Hybrid/in/'
    paths.DATA_IN_DIR = 'D:/Workspace/IIASA/ArcGIS_Scripting_Hybrid_Maps/Cropland Hybrid/in/'
    paths.DATA_OUT_DIR = 'D:/Workspace/IIASA/ArcGIS_Scripting_Hybrid_Maps/Cropland Hybrid/out/'
    
    paths.input = iterableStruct()
    paths.output = iterableStruct()
    paths.input.landMaps = []
    paths.input.landMaps.append(paths.DATA_IN_DIR + 'mod_eu1')
    paths.input.landMaps.append(paths.DATA_IN_DIR + 'glob_eu1')
    paths.input.landMaps.append(paths.DATA_IN_DIR + 'glc_eu')
    
    paths.input.dissimilarityMatrixFile = 'dissimilarity_matrix.txt'
    
    paths.output.hybridMap = paths.DATA_OUT_DIR + 'hybrid.img'
    
    ProduceHybridMap(gui, paths = paths)
