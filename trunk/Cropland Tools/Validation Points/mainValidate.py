# -*- coding: utf-8 -*-
"""
Description:    Tool for checking consistency of dataset with a set of
                validation points

Created:        Tuesday, Jun 12, 2012

Author:         AndriyBun
"""

from parseExcel import parseValidationPointsExcel
#from calculateDiff import calculateDiff

if __name__ == "__main__":
    
    validationPoints = parseValidationPointsExcel("..\\Data\\validation_points.xls")
    
    numVP = len(validationPoints)
    
    print numVP
    
