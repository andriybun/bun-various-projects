import sys
import os
import warnings
commonDir = os.path.dirname(sys.argv[0]) + '/../Common'
sys.path.append(commonDir)

from map import map


class landCoverClassMap(map):
    
    def __init__(self, fileName = None):
        self.__data__ = []
        if fileName is not None:
            file = open(fileName, 'r')
            rawData = file.readlines()
            for line in rawData:
                # skip empty lines and lines starting with '#'
                if (len(line) == 1) or (line[0] == '#'):
                    continue
                row = line.split('\t')
                try:
                    row.remove('')
                except:
                    pass
                key = int(row[0])
                aggregatedVal = int(row[1])
                self.insert(key, aggregatedVal)

#if __name__ == "__main__":
#    lcmap = landCoverClassMap('glc_mapping.txt')
#    lcmap.insert(67, 65)
#    print lcmap.__data__