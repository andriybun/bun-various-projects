# Date:   01/09/2010
# Author: Andriy Bun
# Name:   Process level function

import sys
import os
commonDir = os.path.dirname(sys.argv[0]) + '\\..\\Common'
sys.path.append(commonDir)

import math
import arcgisscripting

from iterableStruct import iterableStruct
from common import CompareNe

EPSILON = 0

def processLevel(paramsStruct, pathsAndUtilities, minMaxClass, unitsName, result, gui):
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1 # allow overwriting rasters:

    tmp = pathsAndUtilities.tmp
    inputsClipped = pathsAndUtilities.inputsClipped

    cell_area = inputsClipped.cell_area
    statLayer = inputsClipped.statLayer
    mark_high32name = inputsClipped.mark_high_32name
    mark_high32 = inputsClipped.mark_high_32
    
    units = paramsStruct.clippedInputs + unitsName
    unitsName = os.path.splitext(unitsName)[0]
    
    minClass = minMaxClass.minClass
    maxClass = minMaxClass.maxClass

    # verifying if the necessary rasters exist:
    inputs = iterableStruct()
    inputs.units = units
    inputs.mark_high32 = mark_high32
    inputs.cell_area = cell_area
    inputs.statLayer = statLayer

    pathsAndUtilities.verifyRasters(inputs, gui)

    # some temporary rasters, used for calculations
    cell_area_min = inputsClipped.cell_area_min
    OutClass      = tmp.OutClass
    OutRaster1    = tmp.OutRaster1
    OutRaster2    = tmp.OutRaster2
    combined      = tmp.combined
    
    combinedName = os.path.splitext(os.path.basename(combined))[0]
    gui.PrintText(units)
    #===============================================================================
    # calculations
    #===============================================================================
    gui.PrintTextTime('Preparing inputs')
    gp.ResetEnvironments()
    gp.Int_sa(units, combined)
    try:
        gp.BuildRasterAttributeTable_management(combined, "OVERWRITE")
    except:
        gui.Warning('Warning! BuildRasterAttributeTable_management failed')
    # create raster of minimum areas:
    gp.Float_sa(statLayer, OutRaster2)
    gp.Divide_sa(OutRaster2, 100, OutRaster1)
    gp.Times_sa(OutRaster1, cell_area, cell_area_min)
    gui.PrintText('Stage 1. Preparing national areas')
    gui.InitialiseStepProgressor('Preparing national areas')

    # loop by all the indeces within the range (minClass, maxClass) in reverse order
    for i in range(maxClass, minClass - 1, -1):
        # raster containing the result of calculations for current index
        OutRaster = OutClass % i
        # selecting only cells with the values that correspond to current index
        gp.Con_sa(mark_high32, cell_area_min, OutRaster1, 0, "VALUE = " + str(i))
        # zonal sum => calculating total area by countries
        gp.ZonalStatistics_sa(combined, "Value", OutRaster1, OutRaster2, "SUM", "DATA")
        gp.Int_sa(OutRaster2, OutRaster1)
        # creating a combined raster of 
        gp.Combine_sa("\'" + combined + "\';\'" + OutRaster1 + "\'", OutRaster)
        gui.SetProgress(100. * (maxClass - i) / max(maxClass - minClass, 1))

    gui.PrintTextTime('Finished')

    gp.BuildRasterAttributeTable_management(combined, "OVERWRITE")

    gui.PrintText('Stage 2. Creating combined raster')
    gui.InitialiseStepProgressor('Creating combined raster')
    for i in range(maxClass, minClass - 1, -1):
#    for i in range(30, minClass - 1, -1):
        OutRaster = OutClass % i
        gui.PrintText('** class %d' % i)
        # Adding a new field for the result:
        gp.addfield (combined, "CLASS_" + str(i),"LONG", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")
#        gui.PrintText('\tadd field')
        # Creating cursor:
        rowsClasses = gp.SearchCursor(OutRaster, "", "", "", combinedName)
#        gui.PrintText('\tsearch cursor')
        rowClasses = rowsClasses.next()
#        gui.PrintText('\trow classes next')
        rowsCombined = gp.UpdateCursor(combined, "", "", "", "VALUE")
#        gui.PrintText('\tupdate cursor')
        rowCombined = rowsCombined.next()
        while rowClasses:
#            gui.PrintText(str(rowCombined.VALUE) + " --> " + str(rowClasses.getValue(combinedName)))
            while rowCombined.VALUE != rowClasses.getValue(combinedName):
                rowCombined = rowsCombined.next()
            if (math.fabs(rowCombined.VALUE - rowClasses.getValue(combinedName)) > 1):
                gui.Warning("Warning! %d != %d" % (rowCombined.VALUE, rowClasses.getValue(combinedName)))
            rowCombined.setValue("CLASS_" + str(i), rowClasses.TMP1)
            rowsCombined.UpdateRow(rowCombined)
            rowClasses = rowsClasses.next()
            rowCombined = rowsCombined.next()
        del rowCombined
        del rowsCombined
        del rowClasses
        del rowsClasses
#        gp.delete_management(OutRaster)
        gui.SetProgress(100. * (maxClass - i) / max(maxClass - minClass, 1))
    gui.PrintTextTime('Finished')

    gui.PrintText('Stage 3. Processing combined raster')
    gui.InitialiseDefaultProgressor('Cropland validator working...')
    count = []
    croplandClasses = []

    rows = gp.SearchCursor(combined,"","","","VALUE")
    # Copy table to 2D array:
    row = rows.next()
    rowID = 0
    while row:
        count.append(row.getValue("VALUE"))
        croplandClasses.append([])
        for i in range(minClass - 1, maxClass):
            croplandClasses[rowID].append(row.getValue("CLASS_" + str(i + 1)))
        row = rows.next()
        rowID += 1
    del row
    del rows

    resSums = []
    resIDs = []

    for i in range(0, len(count)):
        natSum = 0
        resSum = 0
        absDiff = count[i]
        resIndex = maxClass
        for j in range(maxClass, minClass-1, -1):
            natSum += croplandClasses[i][j]
            tmpDiff = abs(natSum - count[i])
            if (tmpDiff <= absDiff):
                absDiff = tmpDiff
                resSum = natSum
                resIndex = j
        resSums.append(resSum)
        resIDs.append(resIndex)

    gui.PrintTextTime('Finished')

    gui.PrintText('Stage 4. Altering combined attribute table')

    gp.BuildRasterAttributeTable_management(combined,"OVERWRITE")

    gp.addfield (combined,"BEST_SUM","LONG", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")
    gp.addfield (combined,"BEST_CLASS","LONG", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")

    rows = gp.UpdateCursor(combined,"","","","VALUE")

    row = rows.next()

    i = 0
    while row:
        while row.VALUE != count[i]:
            row = rows.next()
        row.setValue("BEST_SUM", resSums[i])
        row.setValue("BEST_CLASS", resIDs[i])
        rows.UpdateRow(row)
        row = rows.next()
        i += 1

    del row
    del rows

    gui.PrintTextTime('Finished')

    gui.PrintText('Stage 5. Compiling final raster')

    gui.InitialiseDefaultProgressor('Cropland validator working...')
    gp.SingleOutputMapAlgebra_sa(combined + ".BEST_CLASS", OutRaster2)

    # gui.PrintText('%s\n%s' % (mark_high32, OutRaster2))
    gp.Combine_sa("\'" + mark_high32 + "\';\'" + OutRaster2 + "\'", OutRaster1)
    
    gp.Con_sa(OutRaster1, cell_area_min, result, "#", mark_high32name + " > 0 AND " + mark_high32name + " >= TMP2")

    gui.PrintText('Cleanup temporary rasters')
    # pathsAndUtilities.DeleteDir(paramsStruct.tmpDir)
    for i in range(maxClass, minClass - 1, -1):
        OutRaster = OutClass % i
        gp.delete_management(OutRaster)
    
    gui.PrintTextTime('Done')


