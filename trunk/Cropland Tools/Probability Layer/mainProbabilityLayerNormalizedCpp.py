# Date:   20/12/2011
# Author: Andriy Bun
# Name:   ...

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

import subprocess
import shutil
import arcgisscripting
from utils import  IsSameExtent, RasterData

from rasterAgreementTable import invertPriorities, rasterAgreementTable

def ones(size):
    xRes = []
    for xIdx in range(size):
        xRes.append(1)
    return xRes

def vectorToStr(vec):
    return str(vec).replace('[', '').replace(']', '').replace(',', '')

if __name__ == "__main__":
    
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)

    runFileName = "probabilityLayerNormalized.exe"

    # Parse command line arguments:
    num_args = len(sys.argv)
    
    cellAreas = RasterData(sys.argv[1])
    countries = RasterData(sys.argv[2])
    
    croplandLayerStrList = (sys.argv[3].replace("'","")).split(";")
    croplandLayerList = []
    numRasters = len(croplandLayerList)
    passCroplandLayerList = ''
    for i in range(numRasters):
        if (croplandLayerStrList[i][0] == '\''):
            croplandLayerStrList[i] = croplandLayerStrList[i][1:-1]
        croplandLayerList = croplandLayerList + RasterData(croplandLayerStrList[i])
        passCroplandLayerList = passCroplandLayerList + '"%s" ' \
            % (croplandLayerStrList[i].getPath())
    passCroplandLayerList = passCroplandLayerList[0:-1]
    
    # Validate for equal extent
    allRasterList = [cellAreas] + [countries] + croplandLayerList
    gp = arcgisscripting.create()
    if not IsSameExtent(gp, allRasterList):
        raise Exception('Error! Rasters don\'t have same extent')

    # Parse priorities
    prior = sys.argv[4].replace("'","")
    if prior == '#':
        priorityValues = ones(numRasters)
    else:
        priorityList = prior.split(";")
        priorityValues = []
        for priorityStr in priorityList:
            priorityValues.append(int(priorityStr))
    prior = sys.argv[5].replace("'","")
    if prior == '#':
        priorityValues2 = ones(numRasters)
    else:
        priorityList2 = prior.split(";")
        priorityValues2 = []
        for priorityStr in priorityList2:
            priorityValues2.append(int(priorityStr))

    # Invert priority vectors    
    priorityValues = invertPriorities(priorityValues)
    priorityValues2 = invertPriorities(priorityValues2)
    
    selectionThreshold = sys.argv[6]
    
    # Parse results' names:
    resultProb = os.path.splitext(sys.argv[7].replace("'",""))[0]
    resultNameTuple = os.path.splitext(resultProb)
    descriptionFileName = resultNameTuple[0] + ".txt"
    resultAvg       = resultNameTuple[0] + "_avg"
    resultMinAvg    = resultNameTuple[0] + "_minavg"
    resultMin       = resultNameTuple[0] + "_min"
    resultMaxAvg    = resultNameTuple[0] + "_maxavg"
    resultMax       = resultNameTuple[0] + "_max"
    
    # Process priorities:

    agreementTable = rasterAgreementTable(priorityValues, priorityValues2)
    descriptionFile = open(descriptionFileName, 'w')
    agreementTable.PrintToFile(descriptionFile, croplandLayerList)
    descriptionFile.close()
    
    resultDir        = os.path.dirname(resultProb)
    tmpDir           = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True

    executeCommand = '"%s" "%s" "%s" "%s" "%s" "%s" %d %s %s %s %s "%s" "%s" "%s" "%s" "%s" "%s" %s' % ( \
        runFileName, \
        workingDir, \
        resultDir, \
        tmpDir, \
        cellAreas.getPath(), \
        countries.getPath(), \
        numRasters, \
        passCroplandLayerList, \
        vectorToStr(priorityValues), \
        vectorToStr(priorityValues2), \
        vectorToStr(agreementTable.weights), \
        resultProb, \
        resultAvg, \
        resultMinAvg, \
        resultMin, \
        resultMaxAvg, \
        resultMax, \
        selectionThreshold)

#    import arcgisscripting
#    gp = arcgisscripting.create()
#    gp.AddMessage(executeCommand)
#    raise Exception("======================")

    callResult = subprocess.call(executeCommand)

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
    
    if not(callResult == 0):
        raise Exception('Error! Function returned error code %d!' % callResult)
        