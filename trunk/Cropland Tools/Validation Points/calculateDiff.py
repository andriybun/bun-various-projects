# -*- coding: utf-8 -*-
"""
Name:       Function for calculating level of agreement with statistics

Created:    Tuesday, Jun 12, 2012

Author:     AndriyBun

"""

import os
import arcgisscripting
import numpy as np
import csv
from agreementMatrices import agreementMatrices

class calculateDiff:
    def __init__(self, zonesRaster, croplandLayerList, probabilityClassesList):
        self.gp = arcgisscripting.create()
        self.zonesRaster = zonesRaster
        self.croplandLayerList = croplandLayerList
        self.probabilityClassesList = probabilityClassesList
        self.rasterAgreement = []
        self.scoresCalculated = False
    
    def calculateDiff(self, validationPoints):
        """
        validationPoints - a list returned by parseValidationPointsExcel() method. Each item in this list
                           must be a class with the following fields: x, y, crPerc, where x - x coordinate
                           y - y coordinate, crPercentage - is the cropland percentage for the reported 
                           validation point
        """

        #TODO: Add progress bar
        #TODO: add numpy to a project
        idx = 0
        for raster in self.croplandLayerList:
            self.rasterAgreement.append(agreementMatrices(len(self.probabilityClassesList)))
            for vp in validationPoints:
                coords = "%f %f" % (vp.x, vp.y)
                croplandValue = self.gp.GetCellValue_management(raster, coords)
                zoneId = self.gp.GetCellValue_management(self.zonesRaster, coords)
                if (croplandValue == 'NoData') and (zoneId != 'NoData'):
                    croplandValue = 0
                elif (zoneId == 'NoData'):
                    continue
                zoneId = int(float(zoneId))
                self.rasterAgreement[idx].addPoint(zoneId, self.getClass(float(croplandValue)), self.getClass(vp.crPerc))
                self.gp.AddMessage('%s\t|  %f\t|  %f\t|  %f' % (coords, vp.crPerc, float(croplandValue), self.getClass(croplandValue)))
            idx += 1
        
    def getClass(self, val):
        cl = 0
        while (val > self.probabilityClassesList[cl]):
            cl += 1
            if cl >= len(self.probabilityClassesList):
                break
        return cl
        
    def getScores(self):
        self.numRasters = len(self.croplandLayerList)
        self.numZones = len(self.rasterAgreement[0].agreementMap)
        self.zonesIdList = self.rasterAgreement[0].agreementMap.keys()
        
        numRasters = self.numRasters
        numZones = self.numZones
        
        self.rasterZonesStats = np.zeros(shape = (numRasters, numZones))
        for rasterIdx in range(numRasters):
            zonesAgreementMapList = self.rasterAgreement[rasterIdx].agreementMap.values()
            for zoneIdx in range(numZones):
                self.rasterZonesStats[rasterIdx, zoneIdx] = \
                    zonesAgreementMapList[zoneIdx].diagonal().sum() / zonesAgreementMapList[zoneIdx].sum()
            
    def printScores(self, resultFile):
        if not self.scoresCalculated:
            self.getScores()
            self.scoresCalculated = True
            
        csvfile = open(resultFile, 'wb')
        zoneScores = csv.writer(csvfile, delimiter=',', quotechar='\'', quoting=csv.QUOTE_MINIMAL)

        rasterNames = ['Country ID']
        for raster in self.croplandLayerList:
            rasterNames.append(os.path.splitext(os.path.basename(raster))[0])
        
        zoneScores.writerow(rasterNames)
        
        for zoneIdx in range(self.numZones):
            lineToWrite = [int(self.zonesIdList[zoneIdx])]
            for rasterIdx in range(self.numRasters):
                lineToWrite.append(self.rasterZonesStats[rasterIdx, zoneIdx])
            zoneScores.writerow(lineToWrite)
        
        csvfile.close()