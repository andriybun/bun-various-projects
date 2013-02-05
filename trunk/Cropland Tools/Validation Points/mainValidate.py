# -*- coding: utf-8 -*-
"""
Description:    Tool for checking consistency of dataset with a set of
                validation points

Created:        Tuesday, Jun 12, 2012

Author:         AndriyBun
"""

"""
    Some initializations
"""

CROPLAND_CLASS_IDX = 4
#probabilityClassesList = [0, 50, 100]


"""
    Code
"""

import sys
import os
import shutil
import warnings
import arcgisscripting
import numpy as np
import csv

from agreementMatrices import agreementMatrices

def list2str(pathList, quote = '\''):
    strList = "%s%s%s" % (quote, pathList[0], quote)
    for path in pathList[1:]:
        strList += ";%s%s%s" % (quote, path, quote)
    return strList

class interfaceT:
    def __init__(self):
        pass
    
    def AddMessage(self, msg):
        print msg
    
    def AddWarning(self, msg):
        warnings.warn(msg, RuntimeWarning)
        
def getScores(croplandLayerList, rasterAgreement):
    numRasters = len(croplandLayerList)
    numZones = len(rasterAgreement[0].agreementMap)
    rasterZonesStats = np.zeros(shape = (numRasters, numZones))
    for rasterIdx in range(numRasters):
        rasterZonesStats[rasterIdx, :] = rasterAgreement[rasterIdx].getScores()
    return rasterZonesStats
        
def printScores(resultFile, croplandLayerList, rasterAgreement, rasterZonesStats):
    numRasters = len(croplandLayerList)
    numZones = len(rasterAgreement[0].agreementMap)
    zonesIdList = rasterAgreement[0].agreementMap.keys()
    
    csvfile = open(resultFile, 'wb')
    zoneScores = csv.writer(csvfile, delimiter=',', quotechar='\'', quoting=csv.QUOTE_MINIMAL)

    rasterNames = ['Country ID', 'Validation points per zone']
    for raster in croplandLayerList:
        rasterNames.append(os.path.splitext(os.path.basename(raster))[0])
    
    zoneScores.writerow(rasterNames)
    
    for zoneIdx in range(numZones):
        lineToWrite = [int(zonesIdList[zoneIdx])]
        lineToWrite.append(rasterAgreement[0].vpPerZone[zoneIdx])
        for rasterIdx in range(numRasters):
            lineToWrite.append(rasterZonesStats[rasterIdx, zoneIdx])
        zoneScores.writerow(lineToWrite)
    
    csvfile.close()        
        
if __name__ == "__main__":
#    interface = arcgisscripting.create()
    interface = interfaceT()
    interface.AddMessage("Running validation points check.")
    workingDir = os.path.dirname(sys.argv[5])
    os.chdir(workingDir)    
    zonesRaster = sys.argv[1]
    inRastersList = (sys.argv[2].replace("'","")).split(";")
    numRasters = len(inRastersList)
    inExcelPath = sys.argv[3]
    probabilityClassesList = map(int, (sys.argv[4].replace(" ","")).split(','))
    output = sys.argv[5]
    
    tmpDir           = workingDir + "\\tmp_" + os.getenv('COMPUTERNAME')
    deleteTmpDir = False
    if not os.path.exists(tmpDir):
        os.mkdir(tmpDir)
        deleteTmpDir = True
    else:
        shutil.rmtree(tmpDir)
        os.mkdir(tmpDir)
    
    pointsTablePath = tmpDir + "\\" + os.path.splitext(os.path.basename(inExcelPath))[0] +  ".dbf"
    pointsTableEvent = tmpDir + "\\" + os.path.splitext(os.path.basename(inExcelPath))[0] +  "_event.dbf"
    pointsTableShp = tmpDir + "\\" + os.path.splitext(os.path.basename(inExcelPath))[0] +  "_shape.shp"
    pointsTableShpTmp = tmpDir + "\\" + os.path.splitext(os.path.basename(inExcelPath))[0] +  "_shape_tmp.shp"
    combinedRastersPath = tmpDir + "\\combinedCropland.img"
    combinedPoints = tmpDir + "\\combinedPoints.shp"
    combinedPointsIdx = tmpDir + "\\%02d\\combinedPoints_%02d.shp"
    
    newFieldNamePattern = "rast%02d"
    countriesFieldName = "CounryID"
    
    gp = arcgisscripting.create()
    gp.workspace = tmpDir

    gp.TableToGeodatabase(inExcelPath, tmpDir)
    gp.MakeXYEventLayer(pointsTablePath, "x", "y", pointsTableEvent)
    gp.CopyFeatures_management(pointsTableEvent, pointsTableShp)
    gp.ExtractValuesToPoints_sa(pointsTableShp, zonesRaster, combinedPoints, "NONE", "VALUE_ONLY")
    gp.AddField_management(combinedPoints, countriesFieldName, "long")
    gp.CalculateField_management(combinedPoints, countriesFieldName, "[RASTERVALU]")
    gp.DeleteField_management(combinedPoints, "RASTERVALU")

    gp.Copy_management(pointsTableShp, pointsTableShpTmp)
    gp.JoinField_management(pointsTableShpTmp, "FID", combinedPoints, "FID", countriesFieldName)
    
    fields = gp.ListFields(pointsTablePath)
    listFields = []
    field = fields.next()
    while field:
        listFields.append(field.Name)
        field = fields.next()
    fields = listFields[3:9]
    
    rasterAgreement = []    
    
    idx = 1
    for inRaster in inRastersList:
        combinedPointsRast = combinedPointsIdx % (idx, idx)
        rastDir = os.path.dirname(combinedPointsRast)
        os.mkdir(rastDir)
        gp.workspace = rastDir
        gp.ExtractValuesToPoints_sa(pointsTableShp, inRaster, combinedPointsRast, "NONE", "VALUE_ONLY")
        gp.DeleteField_management(combinedPointsRast, list2str(fields, ''))
        newFieldName = newFieldNamePattern % idx
        gp.AddField_management(combinedPointsRast, newFieldName, "double")
        gp.CalculateField_management(combinedPointsRast, newFieldName, "[RASTERVALU]")
        gp.DeleteField_management(combinedPointsRast, "RASTERVALU")
        gp.JoinField_management(pointsTableShpTmp, "FID", combinedPointsRast, "FID", newFieldName)
        idx += 1
        # Initialize list to store agreement tables
        rasterAgreement.append(agreementMatrices(probabilityClassesList))
        
    gp.workspace = tmpDir
    
    rows = gp.SearchCursor(pointsTableShpTmp, "", "", "", "")
    row = rows.next()
    idx = 1
    
    
        
    while row:
        # Read joined table
        # Read country index
        zoneId = row.GetValue(countriesFieldName)
        # Read land classes and percentages. First we fill two lists to lookup for 
        # a value corresponding to cropland
        landClass = []
        landPerc = []
        for col in range(3):
            landClass.append(row.GetValue(fields[col * 2]))
            landPerc.append(row.GetValue(fields[col * 2 + 1]))
        # Now lookup for cropland and its percentage per cell
        if CROPLAND_CLASS_IDX in landClass:
            crPerc = landPerc[landClass.index(CROPLAND_CLASS_IDX)]
        else:
            crPerc = 0
        # Fill in agreement tables
        for rasterIdx in range(numRasters):
            newFieldName = newFieldNamePattern % (rasterIdx + 1)
            croplandValue = row.GetValue(newFieldName)
            rasterAgreement[rasterIdx].addPoint(zoneId, croplandValue, crPerc)

        row = rows.next()
        idx += 1
    
    # Process collected data and calculate scores for countries
    rasterZonesStats = getScores(inRastersList, rasterAgreement)
    # Print results to file
    printScores(output, inRastersList, rasterAgreement, rasterZonesStats)
    
    if deleteTmpDir:
        shutil.rmtree(tmpDir)
    
    """
    "m:\Andriy\new_run\1_Cropland_Validation_Inputs\countries.img" "m:\Andriy\new_run\1_Cropland_Validation_Inputs\prod_avg_regional_120307_ABmin.img;m:\Andriy\new_run\1_Cropland_Validation_Inputs\prod_avg_modis.img;m:\Andriy\new_run\1_Cropland_Validation_Inputs\prod_avg_geocover.img" "M:\Andriy\new_run\Results_Global\120826_test\validation_points.csv" "M:\Andriy\new_run\Results_Global\120826_test\result_optimized.csv"
    """