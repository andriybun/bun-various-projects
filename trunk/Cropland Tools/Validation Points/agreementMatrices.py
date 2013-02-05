import numpy as np

class agreementMatrices:
    def __init__(self, classes):
        self.classes = classes
        self.numClasses = len(classes)
        self.emptyMatrix = np.zeros(shape = (self.numClasses, self.numClasses))
        self.agreementMap = {}

    def getClass(self, val):
        cl = 0
        while (val > self.classes[cl]):
            cl += 1
            if cl >= len(self.classes):
                break
        return cl

    def addPoint(self, zoneId, givenVal, expectedVal):
        """
        zoneId        - unique key to identify country
        givenClass    - value given in cropland product
        expectedClass - expected value (provided by volunteered data)
        """
        givenClass = self.getClass(givenVal);
        expectedClass = self.getClass(expectedVal);
        if zoneId in self.agreementMap:
            self.agreementMap[zoneId][givenClass][expectedClass] += 1
        else:
            self.agreementMap[zoneId] = np.copy(self.emptyMatrix)
            self.agreementMap[zoneId][givenClass][expectedClass] = 1
    
    def getScores(self):
        zonesAgreementMapList = self.agreementMap.values()
        numZones = len(zonesAgreementMapList)
        rasterZonesStats = np.zeros(shape = (numZones))
        self.vpPerZone = np.zeros(shape = (numZones))
        for zoneIdx in range(numZones):
            rasterZonesStats[zoneIdx] = zonesAgreementMapList[zoneIdx].diagonal().sum() / zonesAgreementMapList[zoneIdx].sum()
            self.vpPerZone[zoneIdx] = zonesAgreementMapList[zoneIdx].sum()
        return rasterZonesStats
        
        
if __name__ == '__main__':
    probabilityClassesList = [0, 50, 100]
    am = agreementMatrices(probabilityClassesList)
    am.addPoint(0, 0, 0)
    am.addPoint(1, 63, 64)
    am.addPoint(1, 43, 63)
    am.addPoint(2, 33, 74)
    print am.agreementMap[0]
    print am.agreementMap[1]
    print am.agreementMap[2]
    z = np.zeros(shape = (2, 3))
    z[1, :] = am.getScores()
    print z