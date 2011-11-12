#===============================================================================
# Zonal statistics replacement
#===============================================================================
import sys
from common import *

if __name__ == "__main__":
    inZoneRaster   = sys.argv[1]
    inValueRaster  = sys.argv[2]
    outRaster      = sys.argv[3]
    if len(sys.argv) > 4:
        statisticsType = sys.argv[4]
    else:
        statisticsType = None
    ZonalStatistics(inZoneRaster, inValueRaster, outRaster, statisticsType)