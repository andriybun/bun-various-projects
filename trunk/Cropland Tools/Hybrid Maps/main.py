# -*- coding: utf-8 -*-
"""
Created on Fri May 13 13:39:39 2011

@author: Andriy Bun
"""

import sys
sys.path.append('..\\Common')

from iterableStruct import iterableStruct
from GeoprocessingInfo import GeoprocessingInfo, GeoprocessingInfo_debug

from ProduceHybridMap import ProduceHybridMap

if __name__ == '__main__':

#    gui = GeoprocessingInfo()
    gui = GeoprocessingInfo_debug()
    
    #inputPaths.croplandLayerList = sys.argv[3].split(";")
    #for i in range(len(inputPaths.croplandLayerList)):
    #    inputPaths.croplandLayerList[i] = inputPaths.croplandLayerList[i][1:-1]
    
    paths = iterableStruct()
    paths.input = iterableStruct()
    paths.input.landMaps = []
    paths.input.landMaps.append('C:\\ProgramFiles\\Cropland Hybrid\\in\\mod_eu1')
    paths.input.landMaps.append('C:\\ProgramFiles\\Cropland Hybrid\\in\\glob_eu1')
    paths.input.landMaps.append('C:\\ProgramFiles\\Cropland Hybrid\\in\\glc_eu')
    
    paths.input.dissimilarityMatrixFile = 'dissimilarity_matrix.txt'
    
    ProduceHybridMap(gui, paths = paths)
    

    
    
    
    
    
