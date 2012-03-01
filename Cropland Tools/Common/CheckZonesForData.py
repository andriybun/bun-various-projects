import sys
import os
import shutil

from ctypes import *

# TODO: accept and parse command line params

if __name__ == "__main__":
    
    # Load DLL
    testDll = cdll.LoadLibrary("gisToolsInterface.dll")

    # Identify the in / out parameters of the function from DLL we will use
    testDll.checkZonesForData.restype = c_int
    testDll.checkZonesForData.argtypes = [ \
        c_int, \
        (c_char_p * numRasters), \
        c_char_p, \
        c_char_p, \
        c_int]

    # Parse input params
    zoneRasterPath = os.path.splitext(sys.argv[1].replace("'",""))[0]

    croplandLayerList = (sys.argv[2].replace("'","")).split(";")
    numRasters = len(croplandLayerList)
    passCroplandLayerList = (c_char_p * numRasters)()
    for i in range(numRasters):
        if (croplandLayerList[i][0] == '\''):
            croplandLayerList[i] = croplandLayerList[i][1:-1]
        passCroplandLayerList[i] = (os.path.splitext(croplandLayerList[i])[0])

    selectionThreshold = int(sys.argv[3])
    
    resultDir = os.path.dirname(croplandLayerList[0])
    tmpDir    = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True
    
    testDll.checkZonesForData( \
        numRasters, \
        passCroplandLayerList, \
        zoneRasterPath, \
        tmpDir, \
        selectionThreshold \
        )

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
