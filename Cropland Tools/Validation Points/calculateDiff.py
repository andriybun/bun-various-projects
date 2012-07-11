# -*- coding: utf-8 -*-
"""
Name:       Function for calculating level of agreement with statistics

Created:    Tuesday, Jun 12, 2012

Author:     AndriyBun

"""

import arcgisscripting

class calculateDiff:
    def __init__(self, croplandLayerList):
        gp = arcgisscripting.create()
        self.croplandLayerList = croplandLayerList
    
    def calculateDiff(self, validationValue, coords):
        """
        validationValue - value of cropland percentage reported for the validation point with 
                          the following coordinates
        coords          - coordinates (string "lat lon")
        """
        croplandValuesList = []
        diff = 0
        for raster in self.croplandLayerList:
            croplandValue = self.gp.GetCellValue_management(raster, coords)
            croplandValuesList.append(croplandValue)
            diff += (croplandValue - validationValue)**2
        return diff