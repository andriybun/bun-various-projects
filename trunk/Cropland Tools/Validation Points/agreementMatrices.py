import numpy as np

class agreementMatrices:
    def __init__(self, numClasses):
        self.numClasses = numClasses
        self.emptyMatrix = np.zeros(shape = (self.numClasses, self.numClasses))
        self.agreementMap = {}

    def addPoint(self, zoneId, givenClass, expectedClass):
        """
        zoneId        - unique key to identify country
        givenClass    - value given in cropland product
        expectedClass - expected value (provided by volunteered data)
        """
        givenClass -= 1;
        expectedClass -= 1;
        if zoneId in self.agreementMap:
            self.agreementMap[zoneId][givenClass][expectedClass] += 1
        else:
            self.agreementMap[zoneId] = np.copy(self.emptyMatrix)
            self.agreementMap[zoneId][givenClass][expectedClass] = 1
    
if __name__ == '__main__':
    am = agreementMatrices(5)
    am.addPoint(1, 3, 4)
    am.addPoint(1, 3, 4)
    am.addPoint(1, 3, 3)
    am.addPoint(2, 3, 4)
    print am.agreementMap[1]
    print am.agreementMap[2]