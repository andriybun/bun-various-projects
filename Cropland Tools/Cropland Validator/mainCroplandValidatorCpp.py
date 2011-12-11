'''
# Name:     Python launcher for cropland validator C++ code
# Created:  10/12/2011
# Author:   Andriy Bun, andr.bun@gmail.com
# Modified: 
'''

import sys
import os
import shutil
import subprocess

if __name__ == "__main__":
    runFileName = "croplandValidator.exe"
    
    areaGrid         = os.path.splitext(sys.argv[1])[0]
    statisticsLevel0 = os.path.splitext(sys.argv[2])[0]
    statisticsLevel1 = os.path.splitext(sys.argv[3])[0]
    statisticsLevel2 = os.path.splitext(sys.argv[4])[0]
    probabilityGrid  = os.path.splitext(sys.argv[5])[0]
    statLayer        = os.path.splitext(sys.argv[6])[0]
    output           = os.path.splitext(sys.argv[7])[0]
    
    resultDir        = os.path.dirname(output)
    tmpDir           = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True
        
    executeCommand = '"%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s"' % ( \
        runFileName, \
        resultDir, \
        tmpDir, \
        areaGrid, \
        statisticsLevel0, \
        statisticsLevel1, \
        statisticsLevel2, \
        probabilityGrid, \
        statLayer, \
        output)

    subprocess.call(executeCommand)

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
