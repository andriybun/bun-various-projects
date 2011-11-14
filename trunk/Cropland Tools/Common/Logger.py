import os

class Logger():
    def __init__(self, FileName):
        self.FileName = FileName
#        if os.path.exists(self.FileName):
#            os.remove(self.FileName)

    def __del__(self):
        os.remove(self.FileName)
        
    def AddMessage(self, MessageStr):
        if os.path.exists(self.FileName):
            logFile = open(self.FileName, 'a')
        else:
            logFile = open(self.FileName, 'w')
        logFile.write(MessageStr + '\n')
        logFile.close()

    