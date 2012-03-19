import subprocess
import sys
import os
import string

if __name__ == "__main__":
    params = string.replace(str(sys.argv[1:]), ',', '')
    params = string.replace(params, '[', '')
    params = string.replace(params, ']', '')
    params = string.replace(params, '\'', '\"')

    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)

    print params
    callResult = subprocess.call('cmd /c "python CheckZonesForData.py %s"' % params)
    
    if not(callResult == 0):
        raise Exception('Error! Function returned error code %d!' % callResult)