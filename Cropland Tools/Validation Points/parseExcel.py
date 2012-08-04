import csv
import exceptions
import string

class vp:
    pass
 
def parseValidationPointsExcel(fileName):
#==============================================================================
#     Returns list of structures with fields:
#       x - longitude coordinate
#       y - latitude coordinate 
#       crPerc - percentage of cropland reported by validator for a given cell   
#==============================================================================
    
    validationSheet = csv.reader(open(fileName, 'rb'), delimiter=';', quotechar='\'')
    
    validationPointsList = []    
    
    # Skip header line    
    validationSheet.next()
    for row in validationSheet:
#    nRows = validationSheet.nrows-1
#    nCols = validationSheet.ncols
#    
#    rawX = validationSheet.col_slice(3, 1)
#    rawY = validationSheet.col_slice(4, 1)
#    
#    rawLc = []
#    rawLcPerc = []
#    rawLc.append(validationSheet.col_slice(7, 1))
#    rawLcPerc.append(validationSheet.col_slice(8, 1))
#    rawLc.append(validationSheet.col_slice(10, 1))
#    rawLcPerc.append(validationSheet.col_slice(11, 1))
#    rawLc.append(validationSheet.col_slice(13, 1))
#    rawLcPerc.append(validationSheet.col_slice(14, 1))

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
