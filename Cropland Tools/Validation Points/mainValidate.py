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
import arcgisscripting

if __name__ == "__main__":
    
    workingDir = os.path.dirname(sys.argv[0])
    os.chdir(workingDir)    
    
    inRastersList = (sys.argv[1].replace("'","")).split(";")
    
    ## CHANGE THIS TO PASSED PATH
    validationPoints = parseValidationPointsExcel("..\\Data\\validation_points.csv")
    
    diffCalculator = calculateDiff(inRastersList)
    diffScores = diffCalculator.calculateDiff(validationPoints)
    diffCalculator.printScores()
    
    