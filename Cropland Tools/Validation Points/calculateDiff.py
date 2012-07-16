# -*- coding: utf-8 -*-
"""
Name:       Function for calculating level of agreement with statistics

Created:    Tuesday, Jun 12, 2012

Author:     AndriyBun

"""

import os
import arcgisscripting

class calculateDiff:
    def __init__(self, croplandLayerList):
        gp = arcgisscripting.create()
        self.croplandLayerList = croplandLayerList
    
    def calculateDiff(self, validationPoints):
        """
        validationPoints - a list returned by parseValidationPointsExcel() method. Each item in this list
                           must be a class with the following fields: x, y, crPerc, where x - x coordinate
                           y - y coordinate, crPercentage - is the cropland percentage for the reported 
                           validation point
        """
        diffScores = []
        self.diffScoresNormalized = []
        
        for raster in self.croplandLayerList:
            diff = 0
            for vp in validationPoints:
                coords = "%f %f" % (vp.x, vp.y)
                croplandValue = self.gp.GetCellValue_management(raster, coords)
                """
                In order to change calculation method of the similarity score - change the next line
                """
                diff += (croplandValue - vp.crPerc)**2
            diffScores.append(diff)

        
        for score in diffScores:
            self.diffScoresNormalized = score / max(diffScores)
        
        return self.diffScoresNormalized
        
    def printScores(self):
        numRasters = len(self.croplandLayerList)
        
        self.gp.AddMessage("%30s - %s" % ("Raster name", "Score (lowest score - best agreement)"))
        for idx in range(numRasters):
            self.gp.AddMessage("%30s - %3.2f" % (os.path.basename(self.croplandLayerList[idx]), \
                self.diffScoresNormalized[idx]))