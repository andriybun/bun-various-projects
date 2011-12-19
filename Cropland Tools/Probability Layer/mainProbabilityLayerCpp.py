# Date:   20/12/2011
# Author: Andriy Bun
# Name:   ...

import sys
import os

from rasterAgreementTable import rasterAgreementTable

def ones(size):
    xRes = []
    for xIdx in range(size):
        xRes.append(1)
    return xRes

if __name__ == "__main__":
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)

    runFileName = "probabilityLayer.exe"

    # Parse command line arguments:
    num_args = len(sys.argv)
    
    cellAreas = sys.argv[1]
    countries = sys.argv[2]
    
    croplandLayerList = sys.argv[3].split(";")
    numRasters = len(croplandLayerList)
    for i in range(numRasters):
        if (croplandLayerList[i][0] == '\''):
            croplandLayerList[i] = croplandLayerList[i][1:-1]

    # Parse priorities
    if sys.argv[4] == '#':
        priorityValues = ones(numRasters)
    else:
        priorityList = sys.argv[4].split(";")
        priorityValues = []
        for priorityStr in priorityList:
            priorityValues.append(int(priorityStr))
    if sys.argv[5] == '#':
        priorityValues2 = ones(numRasters)
    else:
        priorityList2 = sys.argv[5].split(";")
        priorityValues2 = []
        for priorityStr in priorityList2:
            priorityValues2.append(int(priorityStr))
    
    # Parse results' names:
    resultProb = sys.argv[6]
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
    
    