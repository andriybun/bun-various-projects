# Date:   01/09/2010
# Author: Andriy Bun
# Name:   Process level function

import arcgisscripting

from iterableStruct import iterableStruct

def processLevel(paramsStruct, pathsAndUtilities, minMaxClass, unitsName, result, gui):
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1 # allow overwriting rasters:

    tmp = pathsAndUtilities.tmp

    cell_area = pathsAndUtilities.inputsClipped.cell_area
    statLayer = pathsAndUtilities.inputsClipped.statLayer
    mark_high32name = pathsAndUtilities.inputsClipped.mark_high_32name
    mark_high32 = pathsAndUtilities.inputsClipped.mark_high_32
    
    units = paramsStruct.tmpDir + unitsName
    
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
    cell_area_min = tmp.cell_area_min
    OutClass      = tmp.OutClass
    OutRaster1    = tmp.OutRaster1
    OutRaster2    = tmp.OutRaster2
    combined      = tmp.combined

    #===============================================================================
    # calculations
    #===============================================================================
    gui.PrintTextTime('Preparing inputs')
    gp.ResetEnvironments()
    gp.Int_sa(units, combined)
    gp.BuildRasterAttributeTable_management(combined,"OVERWRITE")
    # create raster of minimum areas:
    gp.Float_sa(statLayer, OutRaster2)
    gp.Divide_sa(OutRaster2, 100, OutRaster1)
    gp.Times_sa(OutRaster1, cell_area, cell_area_min)
    gui.PrintTextTime('Stage 1. Preparing national areas')
    gui.InitialiseStepProgressor('Preparing national areas')
    # loop by all the indeces within the range (minClass, maxClass) in reverse order
    for i in range(maxClass, minClass - 1, -1):
        # raster containing the result of calculations for current index
        OutRaster = OutClass + str(i)
        # selecting only cells with the values that correspond to current index
        gp.Con_sa(mark_high32, cell_area_min, OutRaster1, 0, "VALUE = " + str(i))
        # zonal sum => calculating total area by countries
        gp.ZonalStatistics_sa(combined, "Value", OutRaster1, OutRaster2, "SUM", "DATA")
        # creating a combined raster of 
        gp.Combine_sa("\'" + units + "\';\'" + OutRaster2 + "\'", OutRaster)
        gui.SetProgress(100. * (maxClass - i) / (maxClass - minClass ))

    gui.PrintTextTime('Finished')

    gp.BuildRasterAttributeTable_management(combined,"OVERWRITE")

    gui.PrintText('Stage 2. Creating combined raster')
    gui.InitialiseStepProgressor('Creating combined raster')
    for i in range(maxClass, minClass - 1, -1):
        OutRaster = OutClass + str(i)
        # Adding a new field for the result:
        gp.addfield (combined,"CLASS_" + str(i),"LONG", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")
        # Creating cursor:
        rowsClasses = gp.SearchCursor(OutRaster, "", "", "", unitsName)
        rowsCombined = gp.UpdateCursor(combined, "", "", "", "VALUE")
        # Iterating through the table
        rowClasses = rowsClasses.next()
        rowCombined = rowsCombined.next()
        while rowClasses:
            while rowCombined.VALUE != rowClasses.getValue(unitsName):
                rowCombined = rowsCombined.next()
            rowCombined.setValue("CLASS_" + str(i), rowClasses.TMP2)
            rowsCombined.UpdateRow(rowCombined)
            rowClasses = rowsClasses.next()
            rowCombined = rowsCombined.next()
        del rowCombined
        del rowClasses
        del rowsCombined
        del rowsClasses
        gp.delete_management(OutRaster)
        gui.SetProgress(100. * (maxClass - i) / (maxClass - minClass ))
        
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
        resIndex = 29
        absDiff = count[i]
        for j in range(maxClass - minClass, -1, -1):
            natSum += croplandClasses[i][j]
            tmpDiff = abs(natSum - count[i])
            if (tmpDiff <= absDiff):
                absDiff = tmpDiff
                resSum = natSum
                resIndex = j + 1
        resSums.append(resSum)
        resIDs.append(resIndex)

    gui.PrintTextTime('Finished')

    gui.PrintText('Stage 4. Altering combined attribute table')

    gp.BuildRasterAttributeTable_management(combined,"OVERWRITE")

    gp.addfield (combined,"CLOSEST_SUM","LONG", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")
    gp.addfield (combined,"CLOSEST_CLASS","LONG", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")

    rows = gp.UpdateCursor(combined,"","","","VALUE")

    row = rows.next()

    i = 0
    while row:
        while row.VALUE != count[i]:
            row = rows.next()

        row.setValue("CLOSEST_SUM", resSums[i])
        row.setValue("CLOSEST_CLASS", resIDs[i])
        rows.UpdateRow(row)
        row = rows.next()
        i += 1

    del row
    del rows

    gui.PrintTextTime('Finished')

    gui.PrintText('Stage 5. Compiling final raster')
    gui.InitialiseStepProgressor('Compiling final raster')
    gp.Con_sa(mark_high32, "0", OutRaster2, "#", "VALUE >= 0")

    for i in range(maxClass, minClass - 1, -1):
        gp.Con_sa(combined, i, OutRaster1, OutRaster2, "CLOSEST_CLASS = " + str(i))
        gp.CopyRaster_management(OutRaster1, OutRaster2)
        gui.SetProgress(100. * (maxClass - i) / (maxClass - minClass ))

    gui.InitialiseDefaultProgressor('Cropland validator working...')
    
    gp.Combine_sa("\'" + mark_high32 + "\';\'" + OutRaster2 + "\'", OutRaster1)

    gp.Con_sa(OutRaster1, cell_area_min, result, "#", mark_high32name + " > 0 AND " + mark_high32name + " >= TMP2")

    gui.PrintTextTime('Done')
