"""
  This is a template for Python script which is called from ArcMap tool.
  Put xDeleteMe.exe file to the same folder and point to this script while 
  creating tool in ArcMap.
"""

# Some basic packages
import sys
import os
import subprocess
import shutil



if __name__ == "__main__":
    #     
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)

    runFileName = "NEW_PROJECT_TEMPLATE.exe"

    
    """
      The following piece of code may be handy to parse directory names and create
      temporary directory if necessary. Computer name is used to avoid network conflicts
      if two or more computers are running scripts in the same directory on network drive.
    """    
#    resultDir        = os.path.dirname(output)
#    tmpDir           = resultDir + "\\tmp_" + os.getenv('COMPUTERNAME')
#    deleteTmpDir = False
#    if not os.path.exists(tmpDir):
#        os.mkdir(tmpDir)
#        deleteTmpDir = True

    """
      It is necessary to pass to C++ program paths to working directory, directory 
      for results and putting temporary files which will be deleted after executing script.
    """
    executeCommand = '"%s" "%s" "%s" "%s"' % ( \
        runFileName, \
        workingDir, \
        resultDir, \
        tmpDir
        )
    """
      Here include other parameters you want to pass to program executable.
      Note! Don't forget to modify format string as well!
    """
        
    callResult = subprocess.call(executeCommand)

#    if deleteTmpDir:
#        shutil.rmtree(tmpDir)
    
    if not(callResult == 0):
        raise Exception('Error! Function returned error code %d!' % callResult)
        