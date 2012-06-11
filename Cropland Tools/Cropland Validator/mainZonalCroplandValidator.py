'''
# Name:     Python launcher for cropland validator C++ code
# Created:  10/12/2011
# Author:   Andriy Bun, andr.bun@gmail.com
# Modified: 
'''

import sys
import os
import shutil
import subprocess
import arcgisscripting

commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

from common import *

if __name__ == "__main__":
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)
    # runFileName = workingDir + "\\croplandValidator.exe"
    runFileName = "croplandValidator.exe"
    
    areaGrid         = sys.argv[1]
    statisticsLevel0 = sys.argv[2]
    statisticsLevel1 = sys.argv[3]
    statisticsLevel2 = sys.argv[4]
    probabilityGrid  = sys.argv[5]
    statLayer        = sys.argv[6]
    output           = os.path.splitext(sys.argv[9])[0]
    
    resultDir        = os.path.dirname(output)
    tmpDir           = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
	
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True

    clippedAreaGrid         = tmpDir + "\\area"
    clippedStatisticsLevel0 = tmpDir + "\\stat0"
    clippedStatisticsLevel1 = tmpDir + "\\stat1"
    clippedStatisticsLevel2 = tmpDir + "\\stat2"
    clippedProbabilityGrid  = tmpDir + "\\prob"
    clippedStatLayer        = tmpDir + "\\minavgmax"

    gp = arcgisscripting.create()
    conditionalRaster = sys.argv[7]
    zonalCondition = sys.argv[8]
    ConClip(conditionalRaster, areaGrid, zonalCondition, clippedAreaGrid + ".img")
    desc = gp.Describe(clippedAreaGrid + ".img")
    coords = desc.Extent

    ConClip(conditionalRaster, statisticsLevel0, zonalCondition, clippedStatisticsLevel0 + ".img")
    ConClip(conditionalRaster, statisticsLevel1, zonalCondition, clippedStatisticsLevel1 + ".img")
    ConClip(conditionalRaster, statisticsLevel2, zonalCondition, clippedStatisticsLevel2 + ".img")
    gp.Clip_management(probabilityGrid, coords, clippedProbabilityGrid + ".img")

    statTypes = ["min", "minavg", "avg", "maxavg", "max"]
    numStatistics = len(statTypes)
    
    statList = []
    outputList = []
    if statLayer == "#":
        # no raster specified, running for all types of statistics
        for statType in statTypes:
            statList.append(probabilityGrid + "_" + statType)
            outputList.append(probabilityGrid + "_" + statType + "_cropland")
        gp.AddWarning("Warning! The output file name will be ignored. " \
            "Autogenerated file names will be used instead.")
    else:
        # statistics raster specified, running only for it
        statList = [statLayer]
        outputList = [output]

    for idx in range(0, numStatistics):
        gp.Clip_management(statList[idx], coords, clippedStatLayer + ".img")
        
        executeCommand = '"%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s"' % ( \
            runFileName, \
            workingDir, \
            resultDir, \
            tmpDir, \
            clippedAreaGrid, \
            clippedStatisticsLevel0, \
            clippedStatisticsLevel1, \
            clippedStatisticsLevel2, \
            clippedProbabilityGrid, \
            clippedStatLayer, \
            outputList)
    
        os.chdir(workingDir)
        callResult = subprocess.call(executeCommand)

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
        
    if not(callResult == 0):
        raise Exception('Error! Function returned error code %d!' % callResult)