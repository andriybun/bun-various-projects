# -*- coding: utf-8 -*-
"""
Created on Mon Nov 29 15:23:01 2010

@author: Andriy Bun

@name:   
"""

from common import *
from config import config
from utils import *
from rasterAgreementTable import rasterAgreementTable
import arcgisscripting
import os
import time

def RunAll(interface, inputPaths = None, coords = None, priorityValues = None, priorityValues2 = None):
    
    interface.PrintTextTime("Started")

    runConfig = config(interface, inputPaths, coords)
 
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1

    if priorityValues is None:
        priorityValues = []
        for i in range(0, runConfig.num_rasters):
            priorityValues.append(1)
    
    if priorityValues2 is None:
        priorityValues2 = []
        for i in range(0, runConfig.num_rasters):
            priorityValues2.append(1)
    
    agreementTable = rasterAgreementTable(priorityValues, priorityValues2)
    agreementTable.Print(interface, runConfig.paths.inputs.LayerList)
    
    descriptionFile = open(runConfig.paths.description, 'w')
    agreementTable.PrintToFile(descriptionFile, runConfig.paths.inputs.LayerList)
    descriptionFile.close()

    weights = agreementTable.weights

    # Calculations:
    list_of_rasters = []
    list_of_rasters_weights = []
    list_of_rasters_one = []
    list_of_rasters_two = []
    
    # Temporary raster names:
    temp1 = runConfig.paths.tmp.temp1
    temp2 = runConfig.paths.tmp.temp2
    temp3 = runConfig.paths.tmp.temp3
    temp4 = runConfig.paths.tmp.temp4
    temp5 = runConfig.paths.tmp.temp5

    #===============================================================================
    # Creating table:
    #===============================================================================
    interface.PrintTextTime("Computations started...")
    interface.PrintTextTime("Processing input cropland rasters...")
    for i in range(0, runConfig.num_rasters):
        InputName = runConfig.paths.inputs.LayerList[i]
        TmpName = runConfig.paths.tmp.LayerList[i]
        MakeRasterOfValues(gp, InputName, 2**i, weights[i], priorityValues2[i], TmpName)
        list_of_rasters.append(InputName)
        list_of_rasters_weights.append(TmpName)
        list_of_rasters_one.append(AddSuffixToName(TmpName, "_one"))
        list_of_rasters_two.append(AddSuffixToName(TmpName, "_two"))

    interface.PrintTextTime("Computing statistics...")
    # Computing per cell sums of these rasters:
    gp.CellStatistics_sa(MakeListString(list_of_rasters), temp3, "SUM")
    gp.CellStatistics_sa(MakeListString(list_of_rasters_weights), temp1, "SUM")
    runConfig.DeleteRasters(list_of_rasters_weights)
    gp.CellStatistics_sa(MakeListString(list_of_rasters_one), temp2, "SUM")
    runConfig.DeleteRasters(list_of_rasters_one)
    gp.CellStatistics_sa(MakeListString(list_of_rasters_two), temp4, "SUM")
    runConfig.DeleteRasters(list_of_rasters_two)
    # Computing statistics:
    # TODO: Turn off minimum / make separate scripts for statistics
#    gp.CellStatistics_sa(MakeListString(list_of_rasters), temp5, "MINIMUM")
#    gp.Con_sa(temp5, temp5, runConfig.paths.resultMin, "#", "VALUE > 0")
    gp.CellStatistics_sa(MakeListString(list_of_rasters), temp5, "MAXIMUM")
    gp.Con_sa(temp5, temp5, runConfig.paths.resultMax, "#", "VALUE > 0")
    gp.Divide_sa(temp3, temp2, runConfig.paths.resultAvg)
    # Cleanup temporary rasters:
    runConfig.DeleteRasters(list_of_rasters)
    interface.PrintTextTime("Statistics computed.")
    # Formatting the output:
    gp.Int_sa(temp1, runConfig.paths.tmp.sumRast)
    gp.BuildRasterAttributeTable_management(runConfig.paths.tmp.sumRast,"OVERWRITE")
    gp.Int_sa(temp2, runConfig.paths.tmp.sumRastOne)
    gp.BuildRasterAttributeTable_management(runConfig.paths.tmp.sumRastOne,"OVERWRITE")
    gp.Int_sa(temp4, runConfig.paths.tmp.sumRastTwo)
    gp.BuildRasterAttributeTable_management(runConfig.paths.tmp.sumRastTwo,"OVERWRITE")
    # Calculating total area by different classes of cropland in countries:
    # selecting only active cells:
    gp.Con_sa(runConfig.paths.tmp.sumRast, runConfig.paths.inputs.cellAreas, temp1, "#", "VALUE > 0")
    # determining zones within countries:
    gp.Combine_sa("\'" + runConfig.paths.tmp.sumRast + "\';\'" + runConfig.paths.inputs.units + "\'", temp2)
    # calculating zonal sum of cell areas by zones within countries:
    #gp.ZonalStatistics_sa(temp2, "Value", temp1, temp3, "SUM", "DATA")
    ZonalStatistics(temp2, temp1, temp3, "SUM")
    # combining rasters in order to obtain value attribute table with columns
    # "countries", "sum of input rasters", "area by country":
    # 'temp3' field refers to total area of land class by country
    gp.Combine_sa("\'" + runConfig.paths.tmp.sumRast + "\';\'" + runConfig.paths.tmp.sumRastOne + "\';\'" + runConfig.paths.tmp.sumRastTwo + "\';\'" + runConfig.paths.inputs.units + "\';\'" + temp3 + "\'", temp1)
    #===============================================================================
    # Processing table data:
    #===============================================================================
    # field names:
    unitsFieldName = os.path.splitext(os.path.basename(runConfig.paths.inputs.units))[0]
    sumRastFieldName = os.path.splitext(os.path.basename(runConfig.paths.tmp.sumRast))[0]
    sumRastOneFieldName = os.path.splitext(os.path.basename(runConfig.paths.tmp.sumRastOne))[0]
    sumRastTwoFieldName = os.path.splitext(os.path.basename(runConfig.paths.tmp.sumRastTwo))[0]
    areaByUnitsFieldName = os.path.splitext(os.path.basename(temp3))[0]
    # Adding a new field for the result:
    interface.PrintText("started processing table")
    gp.addfield(temp1, "LAND_CLASS","LONG", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")
    # Creating cursor:
    rows = gp.UpdateCursor(temp1, "", "", "", unitsFieldName + "; " + sumRastOneFieldName + "; "  + sumRastTwoFieldName + "; " + areaByUnitsFieldName + " DESC")
    x = 0
    # Getting number of records in the table:
    num = gp.getCount(temp1)
    # Processing the first row:
    row = rows.next()
    currCountry = row.getValue(unitsFieldName)
    currRastAgree = row.getValue(sumRastOneFieldName)
    currRastAgree2 = row.getValue(sumRastTwoFieldName)
    prevCountry = currCountry
    prevRastAgree = currRastAgree
    prevRastAgree2 = currRastAgree2
    row.setValue("LAND_CLASS", agreementTable.FindFirst(currRastAgree, currRastAgree2))
    rows.UpdateRow(row)
    x = x + 1;
    #interface.PrintText(str(x) + ":\t" + str(currCountry) + "\t" + str(currRastAgree) + "\t" + str(currRastAgree2) + "\t" + str(row.getValue(areaByUnitsFieldName)) + "\t" + str(row.getValue("LAND_CLASS")))
    # Processing rows:
    while x < num:
        row = rows.next()
        currCountry = row.getValue(unitsFieldName)
        currRastAgree = row.getValue(sumRastOneFieldName)
        currRastAgree2 = row.getValue(sumRastTwoFieldName)
        if (prevCountry != currCountry) or (prevRastAgree != currRastAgree) or (prevRastAgree2 != currRastAgree2):
            i = 0
        else:
            i += 1
        row.setValue("LAND_CLASS", agreementTable.FindFirst(currRastAgree, currRastAgree2) + i)
        rows.UpdateRow(row)
        x = x + 1;
        prevCountry = currCountry
        prevRastAgree = currRastAgree
        prevRastAgree2 = currRastAgree2
        #interface.PrintText(str(x) + ":\t" + str(currCountry) + "\t" + str(currRastAgree) + "\t" + str(currRastAgree2) + "\t" + str(row.getValue(areaByUnitsFieldName)) + "\t" + str(row.getValue("LAND_CLASS")))
    #===============================================================================
    # Writing results to the output raster
    #===============================================================================
    interface.PrintTextTime('Writing result...')
    gp.ExtractByAttributes_sa(temp1 + ".LAND_CLASS", "LAND_CLASS > 0", runConfig.paths.result)

    del row
    del rows

    time.sleep(1)

    # Deleting temporary rasters:
    interface.PrintTextTime('Done! Deleting temporary rasters')
    gp.Delete_management(temp1)
    gp.Delete_management(runConfig.paths.tmp.sumRast)
    runConfig.DeleteDir(runConfig.paths.TMPDIR)

    interface.PrintTextTime('Finished')
#===============================================================================
# TODO: fix no clip
#===============================================================================
#    runConfig = config(interface, inputPaths, coords = None, doClip = 0)
#    Validate(interface, runConfig, priorityValues, priorityValues2)
