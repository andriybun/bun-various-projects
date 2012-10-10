# -*- coding: utf-8 -*-
"""
Description:    Tool for checking consistency of dataset with a set of
                validation points

Created:        Tuesday, Jun 12, 2012

Author:         AndriyBun
"""

from parseExcel import parseValidationPointsExcel
from calculateDiff import calculateDiff

import sys
import os

if __name__ == "__main__":
    
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)    
    
    zonesRaster = sys.argv[1]
    inRastersList = (sys.argv[2].replace("'","")).split(";")
    inExcelPath = sys.argv[3]
    
    ## CHANGE THIS TO PASSED PATH
    validationPoints = parseValidationPointsExcel(inExcelPath) # "..\\Data\\validation_points.csv"
    
    # TODO: make generic
    probabilityClassesList = [0, 50, 100] # lowerBound < value <= upperBound
    
    diffCalculator = calculateDiff(zonesRaster, inRastersList, probabilityClassesList)
    diffScores = diffCalculator.calculateDiff(validationPoints)
    diffCalculator.printScores(sys.argv[4])