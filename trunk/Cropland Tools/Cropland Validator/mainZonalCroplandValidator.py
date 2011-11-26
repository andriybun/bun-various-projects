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

from common import *
from iterableStruct import iterableStruct
from runAll import runAll
import arcgisscripting
from GeoprocessingInfo import GeoprocessingInfo, GeoprocessingInfo_debug

# pass GUI arguments to variables
inputsNotClipped = iterableStruct()

inputsNotClipped.cell_area        = sys.argv[1]
inputsNotClipped.countries        = sys.argv[2]
inputsNotClipped.subnationalUnits = sys.argv[3]
inputsNotClipped.subregionalUnits = sys.argv[4]
inputsNotClipped.mark_high_32     = sys.argv[5]
inputsNotClipped.statLayer        = sys.argv[6]

# Clipping a particular region
conditionalRaster = sys.argv[7]
zonalCondition = sys.argv[8]
if (conditionalRaster == '#') or (zonalCondition == '#'):
    coords = None
else:
    gp = arcgisscripting.create ()
    gp.AddMessage('Started clipping input rasters')
    clippedLevel0 = GetTmpDir() + "clippedLevel0.img"
    clippedLevel1 = GetTmpDir() + "clippedLevel1.img"
    clippedLevel2 = GetTmpDir() + "clippedLevel2.img"
#    ConClip(conditionalRaster, inputsNotClipped.countries, zonalCondition, clippedLevel0)
#    ConClip(conditionalRaster, inputsNotClipped.subnationalUnits, zonalCondition, clippedLevel1)
#    ConClip(conditionalRaster, inputsNotClipped.subregionalUnits, zonalCondition, clippedLevel2)
    inputsNotClipped.countries = clippedLevel0
    inputsNotClipped.subnationalUnits = clippedLevel1
    inputsNotClipped.subregionalUnits = clippedLevel2
    desc = gp.Describe(inputsNotClipped.countries)
    coords = desc.Extent
    gp.AddMessage('Clipped maps. The resulting extent will be:')
    gp.AddMessage(coords)

# End clipping

output = sys.argv[9]

interface = GeoprocessingInfo()

runAll(interface, coords, inputsNotClipped, output, sys.argv)