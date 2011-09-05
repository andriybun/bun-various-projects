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
from utils import computeBestClass

def ProduceHybridMap(gui, paths):

    # Initialization
    runConfig = config(gui, paths)
    input = paths.input
    tmp = runConfig.paths.tmp
    aggregatedFieldName = "CR_CLASS"
    singleMapNameTemplate = paths.tmp.singleMapNameTemplate
    numMaps = len(input.landMaps)
    
    # Dissimilarity matrix
    dissimilarityMatrix = ReadDissimilarityMatrix(input.dissimilarityMatrixFile)
    
    # initialize mappings
    mapping = []
    for raster in paths.input.landMaps:
        fileName = os.path.splitext(raster)[0] + '.txt'
        mapping.append(landCoverClassMap(fileName))
    
    # Clip input maps
    gui.PrintTextTime('Clipping input maps')
    clippedLandMaps = runConfig.ClipRasters(input.landMaps, \
        tmp.dir, \
        clippedNameTemplate = singleMapNameTemplate + paths.extension)
    
    # Combine all land cover products into one raster
    gui.PrintTextTime('Combining landcover products into one raster')
    productsList = MakeListString(clippedLandMaps)
    runConfig.gp.Combine_sa(productsList, tmp.combinedRaster)
    
    # Processing created combined table
    gui.PrintTextTime('Processing combined table')
    runConfig.gp.addfield(tmp.combinedRaster, aggregatedFieldName, "SHORT", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")
    rows = runConfig.gp.UpdateCursor(tmp.combinedRaster, "", "", "", "")
    row = rows.next()
    aggregatedTable = []
    while row:
        classesInCell = []
        for idx in range(0, numMaps):
            aggregatedClass = mapping[idx].find(row.getValue(singleMapNameTemplate % (idx + 1)))
            if (aggregatedClass == 0 and idx == 1):
                gui.PrintText(str(row.getValue(singleMapNameTemplate % (idx + 1))))
            classesInCell.append(aggregatedClass)
        aggregatedTable.append(classesInCell)
#        gui.PrintText('classes: ' + str(classesInCell))
        listOfClasses = computeBestClass(classesInCell, dissimilarityMatrix)
        # STOPGAP:
        # TODO: implement algorithm:
        bestClass = listOfClasses[0]
        # END STOPGAP
        row.setValue(aggregatedFieldName, bestClass)
#        gui.PrintText(str(listOfClasses))
#        gui.PrintText('%d\t%d\t%d\n' % (row.getValue('map_1'), row.getValue('map_2'), row.getValue('map_3')))
        rows.UpdateRow(row)
        row = rows.next()
    del row
    del rows
    
    for idx in range(0, numMaps):
        runConfig.gp.deletefield(tmp.combinedRaster, singleMapNameTemplate % (idx + 1))
    
    # Saving results
    gui.PrintTextTime('Saving result')
    runConfig.gp.ExtractByAttributes_sa(tmp.combinedRaster + "." + aggregatedFieldName, \
        "%s >= 0" % (aggregatedFieldName), paths.output.hybridMap)
    
    # Delete temporary directory
    gui.PrintTextTime('Deleting temporary directory')
    runConfig.DeleteDir(paths.tmp.dir)