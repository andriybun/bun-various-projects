'''
# Name:     ArcGIS interface for smth
# Created:  30/09/2010
# Author:   Andriy Bun, andr.bun@gmail.com
# Modified: 
'''

import sys
import os
import arcgisscripting

gp = arcgisscripting.create()

# pass GUI arguments to variables
raster1 = sys.argv[1]
raster2 = sys.argv[2]
raster3 = sys.argv[3]
raster4 = sys.argv[4]
outFile = sys.argv[5]

inputsNotClipped.mark_high_32     = paramsStruct.homeDir + paramsStruct.inputDir + "mark_high32"
inputsNotClipped.countries        = paramsStruct.homeDir + paramsStruct.inputDir + "national"
inputsNotClipped.subnationalUnits = paramsStruct.homeDir + paramsStruct.inputDir + "level1_img"
inputsNotClipped.subregionalUnits = paramsStruct.homeDir + paramsStruct.inputDir + "level2"
inputsNotClipped.cell_area        = paramsStruct.homeDir + paramsStruct.inputDir + "a_0083_1000"
inputsNotClipped.statLayer        = paramsStruct.homeDir + paramsStruct.inputDir + "max_5maps-20n"

# results of level analysis
outputs.resultLevel = []
outputs.resultForCalibratedLevel = ['dummy']
outputs.combinedResult = ['dummy']
outputs.resultLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resLevel_0")
outputs.resultLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resLevel_1")
outputs.resultLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resLevel_2")
outputs.resultForCalibratedLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resCalibr_1")
outputs.resultForCalibratedLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resCalibr_2")
outputs.combinedResult.append(paramsStruct.homeDir + paramsStruct.resultDir + "resCombi_1")
outputs.combinedResult.append(paramsStruct.homeDir + paramsStruct.resultDir + "resCombi_2")
outputs.resultStat = outputs.combinedResult[2] + "st"
outputs.result_area = paramsStruct.homeDir + paramsStruct.resultDir + "result_ar"
outputs.result_sum = paramsStruct.homeDir + paramsStruct.resultDir + "result_sm"


# print something on the screen
for i in range(1, 6):
    gp.AddMessage(str(i))