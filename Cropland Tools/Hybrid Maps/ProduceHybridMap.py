# -*- coding: utf-8 -*-
"""
Created on Fri May 13 15:05:37 2011

@author: Andriy Bun
"""

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

from config import config
from landCoverClassMap import landCoverClassMap
from DissimilarityMatrix import ReadDissimilarityMatrix
from common import MakeListString

def ProduceHybridMap(gui, paths):

    # Initialization
    runConfig = config(gui, paths)
    input = paths.input
    tmp = runConfig.paths.tmp
    singleMapNameTemplate = 'map_%d'
    numMaps = len(input.landMaps)
    
    # Dissimilarity matrix
    dissimilarityMatrix = ReadDissimilarityMatrix(input.dissimilarityMatrixFile)
    
    # initialize mappings
    mapping = []
    for raster in paths.input.landMaps:
        fileName = os.path.splitext(raster)[0] + '.txt'
        mapping.append(landCoverClassMap(fileName))
    
    # Clip input maps
    clippedLandMaps = runConfig.ClipRasters(input.landMaps, \
        tmp.DIR, \
        clippedNameTemplate = singleMapNameTemplate + '.img')
    
    # Combine all land cover products into one raster
    productsList = MakeListString(clippedLandMaps)
    runConfig.gp.Combine_sa(productsList, tmp.combinedRaster)
    
    # Creating cursor:
    rows = runConfig.gp.SearchCursor(tmp.combinedRaster, "", "", "", "")
    row = rows.next()
    aggregatedTable = []
    while row:
        parsedRow = []
        for idx in range(0, numMaps):
            parsedRow.append(mapping[idx].find(row.getvalue(singleMapNameTemplate % (idx + 1))))
        
        #for 
        aggregatedTable.append(parsedRow)
        #print '%d\t%d\t%d\n' % (row.getvalue('map_1'), row.getvalue('map_2'), row.getvalue('map_3'))
        row = rows.next()
    #print aggregatedTable
    