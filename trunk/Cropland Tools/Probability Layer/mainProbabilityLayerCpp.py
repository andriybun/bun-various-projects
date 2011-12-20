# Date:   20/12/2011
# Author: Andriy Bun
# Name:   ...

import sys
import os
import subprocess
import shutil

from rasterAgreementTable import rasterAgreementTable

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

    runFileName = "probabilityLayer.exe"

    # Parse command line arguments:
    num_args = len(sys.argv)
    
    cellAreas = sys.argv[1].replace("'","")
    countries = sys.argv[2].replace("'","")
    
    croplandLayerList = (sys.argv[3].replace("'","")).split(";")
    numRasters = len(croplandLayerList)
    passCroplandLayerList = ''
    for i in range(numRasters):
        if (croplandLayerList[i][0] == '\''):
            croplandLayerList[i] = croplandLayerList[i][1:-1]
        passCroplandLayerList = passCroplandLayerList + '"%s" ' % (croplandLayerList[i])
    passCroplandLayerList = passCroplandLayerList[0:-1]

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
    
    # Parse results' names:
    resultProb = sys.argv[6].replace("'","")
    resultNameTuple = os.path.splitext(resultProb)
    descriptionFileName = resultNameTuple[0] + ".txt"
    resultAvg = resultNameTuple[0] + "_avg" + resultNameTuple[1]
    resultMin = resultNameTuple[0] + "_min" + resultNameTuple[1]
    resultMax = resultNameTuple[0] + "_max" + resultNameTuple[1]
    
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
    
    executeCommand = '"%s" "%s" "%s" %d %s %s %s "%s" "%s" "%s" "%s"' % ( \
        runFileName, \
        cellAreas, \
        countries, \
        numRasters, \
        passCroplandLayerList, \
        vectorToStr(priorityValues), \
        vectorToStr(priorityValues2), \
        resultProb, \
        resultAvg, \
        resultMin, \
        resultMax)

    callResult = subprocess.call(executeCommand)

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
        
    if not(callResult == 0):
        raise Exception('Error! Function returned error code %d!' % str(callResult))