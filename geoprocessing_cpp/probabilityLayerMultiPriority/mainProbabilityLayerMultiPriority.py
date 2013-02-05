"""
  This is a template for Python script which is called from ArcMap tool.
  Put probabilityLayerMultiPriority.exe file to the same folder and point to this script while 
  creating tool in ArcMap.
"""

# Some basic packages
import sys
import os
import subprocess
import shutil

from parseCsv import parseCsv



if __name__ == "__main__":
    raise Exception('here')     
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)
    runFileName = "probabilityLayerMultiPriority.exe"

    # Parse command line arguments:
    num_args = len(sys.argv)
    
    cellAreas = os.path.splitext(sys.argv[1].replace("'",""))[0]
    countries = os.path.splitext(sys.argv[2].replace("'",""))[0]
    
    croplandLayerList = (sys.argv[3].replace("'","")).split(";")
    numRasters = len(croplandLayerList)
    passCroplandLayerList = ''
    for i in range(numRasters):
        if (croplandLayerList[i][0] == '\''):
            croplandLayerList[i] = croplandLayerList[i][1:-1]
        passCroplandLayerList = passCroplandLayerList + '"%s" ' \
            % (os.path.splitext(croplandLayerList[i])[0])
    passCroplandLayerList = passCroplandLayerList[0:-1]
    
    # Parse priorities
    csvFileIn = sys.argv[4]    
    
    # Parse results' names:
    resultProb = os.path.splitext(sys.argv[5].replace("'",""))[0]
    resultNameTuple = os.path.splitext(resultProb)
    descriptionFileName = resultNameTuple[0] + ".txt"
    resultAvg       = resultNameTuple[0] + "_avg"
    resultMinAvg    = resultNameTuple[0] + "_minavg"
    resultMin       = resultNameTuple[0] + "_min"
    resultMaxAvg    = resultNameTuple[0] + "_maxavg"
    resultMax       = resultNameTuple[0] + "_max"
    
    # Directories
    resultDir        = os.path.dirname(resultProb)
    tmpDir           = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True

    # CSV file with parsed priorities
    csvFileOut = tmpDir + "\\prior.csv"
    parseCsv(csvFileIn, csvFileOut)
    
    """
      It is necessary to pass to C++ program paths to working directory, directory 
      for results and putting temporary files which will be deleted after executing script.
    """
    executeCommand = '"%s" "%s" "%s" "%s" "%s" "%s" %d %s %s "%s" "%s" "%s" "%s" "%s" "%s"' % ( \
        runFileName, \
        workingDir, \
        resultDir, \
        tmpDir, \
        cellAreas, \
        countries, \
        numRasters, \
        passCroplandLayerList, \
        csvFileOut, \
        resultProb, \
        resultAvg, \
        resultMinAvg, \
        resultMin, \
        resultMaxAvg, \
        resultMax)


    """
      Here include other parameters you want to pass to program executable.
      Note! Don't forget to modify format string as well!
    """
        
    callResult = subprocess.call(executeCommand)

    if deleteTmpDir:
        shutil.rmtree(tmpDir)
    
    if not(callResult == 0):
        raise Exception('Error! Function returned error code %d!' % callResult)
       
    raise Exception('Debug')