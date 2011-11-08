#===============================================================================
# Conditional clip
#===============================================================================
import sys
from common import *

if __name__ == "__main__":
    inConditionalLayer = sys.argv[1]
    inValueLayer       = sys.argv[2]
    condition          = sys.argv[3]
    outLayer           = sys.argv[4]
    ConClip(inConditionalLayer, inValueLayer, condition, outLayer)