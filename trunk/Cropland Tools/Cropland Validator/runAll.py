# Created:  07/09/2010
# Author:   Andriy Bun
# Modified: 
# Name:     Comparison of global cropland cover script modified for use with GUI

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

from iterableStruct import iterableStruct
from utils import utils
from processLevel import processLevel
from calibrateLevel import calibrateLevel
from Logger import Logger

def runAll(gui, coords = None, inputsNotClipped = None, output = None, argv = None):
    gui.PrintTextTime('Calculations started')
#    # coordinates for clipping rasters:
#    if coords is None:
#        coords = "-18 4 0 17"
#        #coords = "-18 -35 52 38"
#        coords = "2 49 8 54" # Benelux
#        coords = "-18 6.6 -10 16.6"

    # initialize paths
    pathsAndUtilities = utils(gui, inputsNotClipped, output);

    if pathsAndUtilities.verifyRasters(pathsAndUtilities.inputsNotClipped, gui) == 1:
        return 1

    # Creating logger object
    logFile = Logger(pathsAndUtilities.logFileName)
    resumeFromPoint = -999
    gui.PrintText(pathsAndUtilities.logFileName)
    
    if os.path.exists(pathsAndUtilities.logFileName):
        existingLogFile = open(pathsAndUtilities.logFile, 'r')
        lines = existingLogFile.readLines()
        if lines[0] == str(argv):
            resumeFromPoint = lines[-1]
        existingLogFile.close()
    logFile.AddMessage(str(argv))

    paramsStruct = pathsAndUtilities.paramsStruct
    inputsNotClipped = pathsAndUtilities.inputsNotClipped
    inputsClipped = pathsAndUtilities.inputsClipped
    outputs = pathsAndUtilities.outputs
    tmp = pathsAndUtilities.tmp

    point = 1
    if point >= resumeFromPoint:
        gui.PrintTextTime('-- Preprocessing rastsers --')
       
        # clip rasters and convert to int(if necessary) 
        pathsAndUtilities.clipRasterInt(inputsNotClipped.countries, inputsClipped.countries, coords)
        pathsAndUtilities.clipRasterInt(inputsNotClipped.subnationalUnits, inputsClipped.subnationalUnits, coords)
        
        pathsAndUtilities.clipRasterInt(inputsNotClipped.subregionalUnits, inputsClipped.subregionalUnits, coords)
        pathsAndUtilities.clipRaster(inputsNotClipped.mark_high_32, inputsClipped.mark_high_32, coords)
        pathsAndUtilities.clipRaster(inputsNotClipped.cell_area, inputsClipped.cell_area, coords)
        pathsAndUtilities.clipRaster(inputsNotClipped.statLayer, inputsClipped.statLayer, coords)

    logFile.AddMessage(str(point))
    point = point + 1

    # minimum and maximum classes of territory in "mark high"
    minMaxClass = iterableStruct()
    minMaxClass.minClass = pathsAndUtilities.gp.GetRasterProperties_management(inputsClipped.mark_high_32, "MINIMUM")
    minMaxClass.maxClass = pathsAndUtilities.gp.GetRasterProperties_management(inputsClipped.mark_high_32, "MAXIMUM")
    gui.PrintText('Min class = %d' % (minMaxClass.minClass))
    gui.PrintText('Max class = %d' % (minMaxClass.maxClass))

    if point >= resumeFromPoint:
        # convert rasters to proper measurement units
        pathsAndUtilities.prepareRasters()

    logFile.AddMessage(str(point))
    point = point + 1
    
    if point >= resumeFromPoint:
        # Processing level 0 (countries):
        gui.PrintTextTime('-- National level --')
        processLevel(paramsStruct, pathsAndUtilities, minMaxClass, inputsClipped.levelStatisticsName[0],
                     outputs.resultLevel[0], gui)
        #verify.checkZonalSums(resultLevel[0])

    logFile.AddMessage(str(point))
    point = point + 1
    
    if point >= resumeFromPoint:
        # Processing level 1 (subnational - regions):
        gui.PrintTextTime('-- Subnational level --')
        processLevel(paramsStruct, pathsAndUtilities, minMaxClass, inputsClipped.levelStatisticsName[1],
                     outputs.resultLevel[1], gui)
        #verify.checkZonalSums(resultLevel[1])

    logFile.AddMessage(str(point))
    point = point + 1

    if point >= resumeFromPoint:
        # calibrate subnational layer
        gui.PrintTextTime('-- Calibrating subnational level --')
        calibrateLevel(paramsStruct, pathsAndUtilities, minMaxClass, 1, gui)
        #verify.checkZonalSums(outputs.combinedResult[1])

    logFile.AddMessage(str(point))
    point = point + 1

    if point >= resumeFromPoint:
        # Processing level 2:
        gui.PrintTextTime('-- Subregional level --')
        processLevel(paramsStruct, pathsAndUtilities, minMaxClass, inputsClipped.levelStatisticsName[2],
                     outputs.resultLevel[2], gui)

    logFile.AddMessage(str(point))
    point = point + 1

    if point >= resumeFromPoint:
        # now we consider the calibrated result for level 1 as a normal result for this level
        pathsAndUtilities.outputs.resultLevel[1] = pathsAndUtilities.outputs.combinedResult[1]
        # calibrate subregional layer
        gui.PrintTextTime('-- Calibrating subregional level --')
        calibrateLevel(paramsStruct, pathsAndUtilities, minMaxClass, 2, gui)
        #verify.checkZonalSums(outputs.combinedResult[2])

    logFile.AddMessage(str(point))
    point = point + 1

    if point >= resumeFromPoint:
        gui.PrintTextTime('-- Processing results --')
        pathsAndUtilities.processResults()

    logFile.AddMessage(str(point))
    point = point + 1
    
    # Delete tmp files
    gui.PrintTextTime('-- Cleanup temporary files --')
    pathsAndUtilities.cleanUp(inputsClipped)
    pathsAndUtilities.cleanUp(tmp)
    
    gui.PrintTextTime('Finished')
