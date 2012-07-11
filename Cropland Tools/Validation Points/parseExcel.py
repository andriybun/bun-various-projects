import xlrd

class vp:
    pass
 
def parseValidationPointsExcel(fileName):
#==============================================================================
#     Returns list of structures with fields:
#       x - longitude coordinate
#       y - latitude coordinate 
#       crPerc - percentage of cropland reported by validator for a given cell   
#==============================================================================
    
    xls_wb = xlrd.open_workbook(fileName)
    validationSheet = xls_wb.sheet_by_name("Validations")
    
    nRows = validationSheet.nrows-1
    nCols = validationSheet.ncols
    
    rawX = validationSheet.col_slice(3, 1)
    rawY = validationSheet.col_slice(4, 1)
    
    rawLc = []
    rawLcPerc = []
    rawLc.append(validationSheet.col_slice(7, 1))
    rawLcPerc.append(validationSheet.col_slice(8, 1))
    rawLc.append(validationSheet.col_slice(10, 1))
    rawLcPerc.append(validationSheet.col_slice(11, 1))
    rawLc.append(validationSheet.col_slice(13, 1))
    rawLcPerc.append(validationSheet.col_slice(14, 1))
    
    validationPointsList = []
    for idx in range(nRows):

        croplIdx = -1
        for cl in range(3):
            if rawLc[cl][idx].value == 4:
                croplIdx = cl
        
        if croplIdx >= 0:        
            curCell = vp()
            curCell.x = rawX[idx].value
            curCell.y = rawY[idx].value
            curCell.crPerc = rawLcPerc[croplIdx][idx].value
            
            validationPointsList.append(curCell)

    return validationPointsList

if __name__ == "__main__":
    vp = parseValidationPointsExcel("..\\Data\\validation_points.xls")
