# Date:   23/02/2009
# Author: Andriy Bun
# Name:   ...

import sys
import os
import arcgisscripting
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

from common import *
from RunAll import RunAll
from iterableStruct import iterableStruct
from GeoprocessingInfo import GeoprocessingInfo

# Parse command line arguments
inputPaths = iterableStruct()

num_args = len(sys.argv)

inputPaths.cellAreas = sys.argv[1]
inputPaths.countries = sys.argv[2]
zonalCondition = sys.argv[3]

# Clipping countries layer according to condition
clippedCountries = GetTmpDir() + "clippedCountries.img"
ConClip(inputPaths.countries, inputPaths.countries, zonalCondition, clippedCountries)
inputPaths.countries = clippedCountries
gp = arcgisscripting.create ()
desc = gp.Describe(inputPaths.countries)
coords = desc.Extent
gp.AddMessage(coords)
# End clipping

inputPaths.croplandLayerList = sys.argv[4].split(";")

for i in range(len(inputPaths.croplandLayerList)):
    if (inputPaths.croplandLayerList[i][0] == '\''):
        inputPaths.croplandLayerList[i] = inputPaths.croplandLayerList[i][1:-1]

# Parse priorities
if sys.argv[5] == '#':
    priorityValues = None
else:
    priorityList = sys.argv[5].split(";")
    priorityValues = []
    for priorityStr in priorityList:
        priorityValues.append(int(priorityStr))

if sys.argv[6] == '#':
    priorityValues2 = None
else:
    priorityList2 = sys.argv[6].split(";")
    priorityValues2 = []
    for priorityStr in priorityList2:
        priorityValues2.append(int(priorityStr))

inputPaths.result = sys.argv[7]

interface = GeoprocessingInfo()
RunAll(interface, inputPaths = inputPaths, coords = coords, priorityValues = priorityValues, priorityValues2 = priorityValues2)

#gp.Delete_management(clippedCountries)