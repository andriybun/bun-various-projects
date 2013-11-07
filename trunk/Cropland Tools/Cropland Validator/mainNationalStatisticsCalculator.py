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
from utils import IsSameExtent

if __name__ == "__main__":
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)
    # runFileName = workingDir + "\\croplandValidator.exe"
    runFileName = "nationalStatisticsCalculator.exe"
    
    areaGrid         = os.path.splitext(sys.argv[1])[0]
    zones            = os.path.splitext(sys.argv[2])[0]
    cropland         = os.path.splitext(sys.argv[3])[0]
    output           = os.path.splitext(sys.argv[4])[0]
    
    # Validate for equal extent
    allRasterList = [areaGrid, zones, cropland]
    gp = arcgisscripting.create()
    if not IsSameExtent(gp, allRasterList):
        raise Exception('Error! Rasters don\'t have same extent')
    
    # Results
    resultDir        = os.path.dirname(output)
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
        areaGrid, \
        zones, \
        cropland, \
        output)

    callResult = subprocess.call(executeCommand)
        
    if not(callResult == 0):
        if deleteTmpDir:
            shutil.rmtree(tmpDir)
        raise Exception('Error! Function returned error code %d!' % callResult)

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
