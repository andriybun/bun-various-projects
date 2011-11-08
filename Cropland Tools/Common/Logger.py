import os

class Logger():
    def __init__(self, FileName):
        self.FileName = FileName
        os.remove(self.FileName)

    def __del__(self):
        os.remove(self.FileName)
        
    def AddMessage(self, MessageStr):
        logFile = open(self.FileName, 'a')
        logFile.write(MessageStr)
        logFile.close()

    