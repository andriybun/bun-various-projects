import csv
import exceptions
#import string
#import arcgisscripting

class vp:
    pass
 
def parseValidationPointsExcel(fileName):
#==============================================================================
#     Returns list of structures with fields:
#       x - longitude coordinate
#       y - latitude coordinate 
#       crPerc - percentage of cropland reported by validator for a given cell   
#==============================================================================
    
    validationSheet = csv.reader(open(fileName, 'rb'), delimiter=',', quotechar='\'')
    
    validationPointsList = []    
    
    # Skip header line    
    validationSheet.next()
    
    for row in validationSheet:
        croplIdx = -1
        for cl in range(3):
            try:
                if int(row[7 + cl * 3]) == 4:
                    croplIdx = cl
            except exceptions.ValueError:
                pass
        
        if croplIdx >= 0:        
            curCell = vp()
            curCell.x = float(row[3].replace(',', '.'))
            curCell.y = float(row[4].replace(',', '.'))
            curCell.crPerc = float(row[8 + croplIdx * 3].replace(',', '.'))
        
            validationPointsList.append(curCell)
    
    return validationPointsList

if __name__ == "__main__":
    vp = parseValidationPointsExcel("..\\Data\\validation_points.csv")
