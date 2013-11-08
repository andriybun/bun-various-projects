# -*- coding: utf-8 -*-
"""
Created on Fri May 13 09:57:26 2011

author: Andriy Bun
name:   Validator of cropland

"""

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

import shutil
import subprocess
import arcgisscripting
from utils import IsSameExtent, RasterData

if __name__ == '__main__':
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)
    runFileName = "validateResults.exe"    

    rasterToValidate    = RasterData(sys.argv[1])
    areaGrid            = RasterData(sys.argv[2])
    nationalStatistics  = RasterData(sys.argv[3])

    # Validate for equal extent
    gp = arcgisscripting.create()
    if not IsSameExtent(gp, [rasterToValidate, areaGrid, nationalStatistics]):
        raise Exception('Error! Rasters don\'t have same extent')

    # Results
    resultDir        = rasterToValidate.getDirPath()
    tmpDir           = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True
    
    executeCommand = '"%s" "%s" "%s" "%s" "%s" "%s" "%s"' % ( \
            runFileName, \
            workingDir, \
            resultDir, \
            tmpDir, \
            rasterToValidate.getPath(), \
            areaGrid.getPath(), \
            nationalStatistics.getPath())
    
    callResult = subprocess.call(executeCommand)
    if not(callResult == 0):
        if deleteTmpDir:
            shutil.rmtree(tmpDir)
        raise Exception('Error! Function returned error code %d!' % callResult)    
