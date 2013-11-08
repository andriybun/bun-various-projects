'''
# Name:     Python launcher for cropland validator C++ code
# Created:  10/12/2011
# Author:   Andriy Bun, andr.bun@gmail.com
# Modified: 
'''

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

import shutil
import subprocess
import arcgisscripting
from utils import IsSameExtent, RasterData

if __name__ == "__main__":
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)
    runFileName = "croplandValidator.exe"
        
    areaGrid         = RasterData(sys.argv[1])
    statisticsLevel0 = RasterData(sys.argv[2])
    statisticsLevel1 = RasterData(sys.argv[3])
    statisticsLevel2 = RasterData(sys.argv[4])
    probabilityGrid  = RasterData(sys.argv[5])
    statLayer        = RasterData(sys.argv[6])
    output           = RasterData(sys.argv[7])
    
    statList = []
    outputList = []
    if statLayer == "#":
        # no raster specified, running for all types of statistics
        statTypes = ["min", "minavg", "avg", "maxavg", "max"]
        numStatistics = len(statTypes)
        for statType in statTypes:
            statList.append(probabilityGrid.getPath() + "_" + statType)
            outputList.append(probabilityGrid.getPath() + "_" + statType + "_cropland")
        gp = arcgisscripting.create()
        gp.AddWarning("Warning! The output file name will be ignored. " \
            "Autogenerated file names will be used instead.")
    else:
        # statistics raster specified, running only for it
        statList.append(statLayer.getPath())
        outputList.append(output.getPath())
        numStatistics = 1
    
    # Validate for equal extent
    allRasterList = [areaGrid, statisticsLevel0, statisticsLevel1, \
                     statisticsLevel2, probabilityGrid, statLayer]
    gp = arcgisscripting.create()
    if not IsSameExtent(gp, allRasterList):
        raise Exception('Error! Rasters don\'t have same extent')
    gp.AddMessage("5")
    # Results
    resultDir        = output.getDirPath()
    tmpDir           = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True
    
    for idx in range(0, numStatistics):
        executeCommand = '"%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s"' % ( \
            runFileName, \
            workingDir, \
            resultDir, \
            tmpDir, \
            areaGrid.getPath(), \
            statisticsLevel0.getPath(), \
            statisticsLevel1.getPath(), \
            statisticsLevel2.getPath(), \
            probabilityGrid.getPath(), \
            statList[idx], \
            outputList[idx])

        callResult = subprocess.call(executeCommand)
        
        if not(callResult == 0):
            if deleteTmpDir:
                shutil.rmtree(tmpDir)
            raise Exception('Error! Function returned error code %d!' % callResult)

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
