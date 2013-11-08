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
    # runFileName = workingDir + "\\croplandValidator.exe"
    runFileName = "nationalStatisticsCalculator.exe"
    
    areaGrid         = RasterData(sys.argv[1])
    zones            = RasterData(sys.argv[2])
    cropland         = RasterData(sys.argv[3])
    output           = RasterData(sys.argv[4])
    
    # Validate for equal extent
    gp = arcgisscripting.create()
    if not IsSameExtent(gp, [areaGrid, zones, cropland]):
        raise Exception('Error! Rasters don\'t have same extent')
    
    # Results
    resultDir        = output.getDirPath()
    tmpDir           = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True
    

    executeCommand = '"%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s"' % ( \
        runFileName, \
        workingDir, \
        resultDir, \
        tmpDir, \
        areaGrid.getPath(), \
        zones.getPath(), \
        cropland.getPath(), \
        output.getPath())

    callResult = subprocess.call(executeCommand)
        
    if not(callResult == 0):
        if deleteTmpDir:
            shutil.rmtree(tmpDir)
        raise Exception('Error! Function returned error code %d!' % callResult)

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
