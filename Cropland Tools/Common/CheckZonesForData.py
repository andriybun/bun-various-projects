import sys
import os
import shutil

import arcgisscripting

from ctypes import *

# TODO: accept and parse command line params

if __name__ == "__main__":
    
#    gp = arcgisscripting.create()
#    gp.addmessage(str(sys.argv))
#    raise Exception('111')
    
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)
    
    # Load DLL
    testDll = cdll.LoadLibrary("gisToolsInterface.dll")

    # Parse input params
    zoneRasterPath = os.path.splitext(sys.argv[1].replace("'",""))[0]

    croplandLayerList = (sys.argv[2].replace("'","")).split(";")
    numRasters = len(croplandLayerList)
    passCroplandLayerList = (c_char_p * numRasters)()
    for i in range(numRasters):
        if (croplandLayerList[i][0] == '\''):
            croplandLayerList[i] = croplandLayerList[i][1:-1]
        passCroplandLayerList[i] = (os.path.splitext(croplandLayerList[i])[0])

    # Identify the in / out parameters of the function from DLL we will use
    testDll.checkZonesForData.restype = c_int
    testDll.checkZonesForData.argtypes = [ \
        c_int, \
        (c_char_p * numRasters), \
        c_char_p, \
        c_char_p, \
        c_float]

    selectionThreshold = float(sys.argv[3])
    
    resultDir = os.path.dirname(croplandLayerList[0])
    tmpDir    = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True
    
    callResult = testDll.checkZonesForData( \
        numRasters, \
        passCroplandLayerList, \
        zoneRasterPath, \
        tmpDir, \
        selectionThreshold \
        )

    if deleteTmpDir:
        shutil.rmtree(tmpDir)

    if not(callResult == 0):
        raise Exception('Error! Function returned error code %d!' % callResult)