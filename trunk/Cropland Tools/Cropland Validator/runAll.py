# Created:  07/09/2010
# Author:   Andriy Bun
# Modified: 
# Name:     Comparison of global cropland cover script modified for use with GUI

from iterableStruct import iterableStruct
from utils import utils
from processLevel import processLevel
from calibrateLevel import calibrateLevel
#from PyQt4 import QtGui, QtCore, Qt

def runAll(gui, coords = None, inputsNotClipped = None, output = None):
    gui.PrintTextTime('Calculations started')
#    # coordinates for clipping rasters:
#    if coords is None:
#        coords = "-18 4 0 17"
#        #coords = "-18 -35 52 38"

    # minimum and maximum classes of territory in "mark high"
    minMaxClass = iterableStruct()
    minMaxClass.minClass = 1
    minMaxClass.maxClass = 31

    # initialize paths
    pathsAndUtilities = utils(gui, inputsNotClipped, output);
    if pathsAndUtilities.verifyRasters(pathsAndUtilities.inputsNotClipped, gui) == 1:
        return 1
    paramsStruct = pathsAndUtilities.paramsStruct
    inputsNotClipped = pathsAndUtilities.inputsNotClipped
    inputsClipped = pathsAndUtilities.inputsClipped
    outputs = pathsAndUtilities.outputs
    tmp = pathsAndUtilities.tmp
    
    gui.PrintTextTime('-- Preprocessing rastsers --')
   
    # clip rasters and convert to int(if necessary) 
    pathsAndUtilities.clipRasterInt(inputsNotClipped.countries, inputsClipped.countries, coords)
    pathsAndUtilities.clipRasterInt(inputsNotClipped.subnationalUnits, inputsClipped.subnationalUnits, coords)
    pathsAndUtilities.clipRasterInt(inputsNotClipped.subregionalUnits, inputsClipped.subregionalUnits, coords)
    pathsAndUtilities.clipRaster(inputsNotClipped.mark_high_32, inputsClipped.mark_high_32, coords)
    pathsAndUtilities.clipRaster(inputsNotClipped.cell_area, inputsClipped.cell_area, coords)
    pathsAndUtilities.clipRaster(inputsNotClipped.statLayer, inputsClipped.statLayer, coords)

    # convert rasters to proper measurement units
    pathsAndUtilities.prepareRasters()
    
    # Processing level 0 (countries):
    gui.PrintTextTime('-- National level --')
    processLevel(paramsStruct, pathsAndUtilities, minMaxClass, inputsClipped.levelStatisticsName[0],
                 outputs.resultLevel[0], gui)
    #verify.checkZonalSums(resultLevel[0])
    
    # Processing level 1 (subnational - regions):
    gui.PrintTextTime('-- Subnational level --')
    processLevel(paramsStruct, pathsAndUtilities, minMaxClass, inputsClipped.levelStatisticsName[1],
                 outputs.resultLevel[1], gui)
    #verify.checkZonalSums(resultLevel[1])
    
    # calibrate subnational layer
    gui.PrintTextTime('-- Calibrating subnational level --')
    calibrateLevel(paramsStruct, pathsAndUtilities, minMaxClass, 1, gui)
    #verify.checkZonalSums(outputs.combinedResult[1])

    # Processing level 2:
    gui.PrintTextTime('-- Subregional level --')
    processLevel(paramsStruct, pathsAndUtilities, minMaxClass, inputsClipped.levelStatisticsName[2],
                 outputs.resultLevel[2], gui)
    
    # now we consider the calibrated result for level 1 as a normal result for this level
    pathsAndUtilities.outputs.resultLevel[1] = pathsAndUtilities.outputs.combinedResult[1]
    # calibrate subregional layer
    gui.PrintTextTime('-- Calibrating subregional level --')
    calibrateLevel(paramsStruct, pathsAndUtilities, minMaxClass, 2, gui)
    #verify.checkZonalSums(outputs.combinedResult[2])
    
    pathsAndUtilities.processResults()
    
    # Delete tmp files
    gui.PrintTextTime('-- Cleanup temporary files --')
    pathsAndUtilities.cleanUp(inputsClipped)
    pathsAndUtilities.cleanUp(tmp)
    
    gui.PrintTextTime('Finished')