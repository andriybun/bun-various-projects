# -*- coding: utf-8 -*-
"""
Created on Fri May 13 15:05:37 2011

@author: Andriy Bun
"""

from config import config
from DissimilarityMatrix import ReadDissimilarityMatrix

def ProduceHybridMap(gui, paths):
    
    runConfig = config(gui, paths)
    input = paths.input
    tmp = runConfig.paths.tmp
    tmp.combinedRaster = tmp.dir + 'combined'
    
    # Combine all land cover products into one raster
    productsList = "'"
    for raster in input.landMaps:
        productsList = productsList + raster +  "';'"
    productsList = productsList[0:-2]
    
    runConfig.gp.Combine_sa(productsList, tmp.combinedRaster)
    
    dissimilarityMatrix = ReadDissimilarityMatrix(input.dissimilarityMatrixFile)
    
    