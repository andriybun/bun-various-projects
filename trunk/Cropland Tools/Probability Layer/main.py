# Date:   23/02/2009
# Author: Andriy Bun
# Name:   ...

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

from RunAll import RunAll
from iterableStruct import iterableStruct
from GeoprocessingInfo import GeoprocessingInfo, GeoprocessingInfo_debug

#===============================================================================
# ArcGIS GUI version
#===============================================================================
# Parse command line arguments
inputPaths = iterableStruct()

num_args = len(sys.argv)

inputPaths.cellAreas = sys.argv[1]
inputPaths.countries = sys.argv[2]
inputPaths.croplandLayerList = sys.argv[3].split(";")

for i in range(len(inputPaths.croplandLayerList)):
    if (inputPaths.croplandLayerList[i][0] == '\''):
        inputPaths.croplandLayerList[i] = inputPaths.croplandLayerList[i][1:-1]

# Parse priorities
if sys.argv[4] == '#':
    priorityValues = None
else:
    priorityList = sys.argv[4].split(";")
    priorityValues = []
    for priorityStr in priorityList:
        priorityValues.append(int(priorityStr))

if sys.argv[5] == '#':
    priorityValues2 = None
else:
    priorityList2 = sys.argv[5].split(";")
    priorityValues2 = []
    for priorityStr in priorityList2:
        priorityValues2.append(int(priorityStr))

coords = sys.argv[6]

inputPaths.result = sys.argv[num_args-2]
inputPaths.resultAvg = sys.argv[num_args-1]

interface = GeoprocessingInfo()
RunAll(interface, inputPaths = inputPaths, coords = coords, priorityValues = priorityValues, priorityValues2 = priorityValues2)

"""
#===============================================================================
# No ArcGIS version
#===============================================================================

# Default equal priorities
priorityValues = [1, 1, 1, 2]
priorityValues2 = [2, 1, 1, 1]

#coords = "-18 -35 52 38" # Africa
coords = None

inputPaths = None

interface = GeoprocessingInfo_debug()
#RunAll(interface, inputPaths = inputPaths, coords = coords, priorityValues = priorityValues, priorityValues2 = priorityValues2)

##===============================================================================
## Validation
##===============================================================================
import os
from utils import *
from config import config

print coords

runConfig = config(interface, inputPaths, coords)

Validate(interface, runConfig, priorityValues = priorityValues, priorityValues2 = priorityValues2)
"""