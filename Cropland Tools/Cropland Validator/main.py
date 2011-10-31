'''
# Name:     ArcGIS interface for smth
# Created:  30/09/2010
# Author:   Andriy Bun, andr.bun@gmail.com
# Modified: 
'''

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

from iterableStruct import iterableStruct
from runAll import runAll
import time
import arcgisscripting
from GeoprocessingInfo import GeoprocessingInfo, GeoprocessingInfo_debug

# pass GUI arguments to variables
inputsNotClipped = iterableStruct()

inputsNotClipped.mark_high_32     = sys.argv[1]
inputsNotClipped.countries        = sys.argv[2]
inputsNotClipped.subnationalUnits = sys.argv[3]
inputsNotClipped.subregionalUnits = sys.argv[4]
inputsNotClipped.cell_area        = sys.argv[5]
inputsNotClipped.statLayer        = sys.argv[6]

coords = sys.argv[7]

output = sys.argv[8]

interface = GeoprocessingInfo()

runAll(interface, coords, inputsNotClipped, output, sys.argv)