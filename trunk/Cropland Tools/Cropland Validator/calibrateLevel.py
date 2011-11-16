# Date:   01/09/2010
# Author: Andriy Bun
# Name:   Process level function

import arcgisscripting, time

from processLevel import processLevel

def calibrateLevel(paramsStruct, pathsAndUtilities, minMaxClass, level, gui):
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1 # allow overwriting rasters:

    inputsClipped = pathsAndUtilities.inputsClipped
    tmp = pathsAndUtilities.tmp
    tmpDir = paramsStruct.tmpDir
    outputs = pathsAndUtilities.outputs

    statisticsLevel1 = paramsStruct.clippedInputs + inputsClipped.levelStatisticsName[level-1]
    statisticsLevel2 = paramsStruct.clippedInputs + inputsClipped.levelStatisticsName[level]

    # temporary rasters
    differName = tmp.differName
    differ = tmp.differ
    zones = tmp.zones
    areaUnits = tmp.areaUnits
    sumLevel1 = tmp.sumLevel1
    sumLevel2 = tmp.sumLevel2
    OutRaster1 = tmp.OutRaster1
    OutRaster2 = tmp.OutRaster2
    OutRaster3 = tmp.OutRaster3

    # preparing zones + calibrated areas
    gui.PrintText('\t\tPreparing zones and calibrated areas')
    # selecting only those areas, with values
    gp.Con_sa(statisticsLevel2, statisticsLevel2, areaUnits, "#", "VALUE > 0")
    gp.BuildRasterAttributeTable_management(areaUnits,"OVERWRITE")
    if gp.getCount(areaUnits) > 0:
        # calculating total area of national level results
        gp.ZonalStatistics_sa(areaUnits, "Value", outputs.combinedResult[level-1], sumLevel1, "SUM", "DATA")
        # calculating total area of subnational level results
        gp.ZonalStatistics_sa(areaUnits, "Value", outputs.resultLevel[level], sumLevel2, "SUM", "DATA")
        # calculating difference between national and subnational levels
        gp.Minus_sa(sumLevel2, sumLevel1, differ)
        # selecting areas where the difference is positive
        gp.Con_sa(differ, 1, OutRaster2, "#", "VALUE >= 0")
        gp.Con_sa(differ, -1, OutRaster3, OutRaster2, "VALUE < 0")
        # replacing 'NoData' values with zeros
        gp.IsNull_sa(OutRaster3, OutRaster2)
        gp.Con_sa(OutRaster2, OutRaster3, OutRaster1, 0, "VALUE = 0")
        gp.Con_sa(statisticsLevel1, OutRaster1, zones, "#", "VALUE >= 0")
        # Now we have a layer, dividing the whole map into 3 zones:
        #  0 - no data on subnational level available; we take the results of national analysis
        #  1 - sum on subnational level is higher; we take the results of subnational analysis
        # -1 - sum on national level is higher; we run the algorithm again for calibrated numbers
        # Check whether there are areas for which calibration is needed
        rows = gp.SearchCursor(zones, "", "", "", "")
        row = rows.next()
        doCalibration = False
        while row:
            if row.getValue('Value') == -1:
                doCalibration = True
            row = rows.next()
        del row
        del rows
        # If calibration is needed:    
        if doCalibration:
            gui.PrintText('\t\tCalibrating areas')
            # calculating national totals for zones with difference >= 0
            gp.Con_sa(zones, outputs.combinedResult[level-1], OutRaster1, "#", "VALUE = 1")
            gp.Con_sa(OutRaster1, inputsClipped.cell_area_min, OutRaster2, "#", "VALUE > 0")
            gp.ZonalStatistics_sa(statisticsLevel1, "Value", OutRaster2, sumLevel1, "SUM", "DATA")
            # replacing 'NoData' values with zeros
            gp.IsNull_sa(sumLevel1, OutRaster2)
            gp.Con_sa(OutRaster2, sumLevel1, OutRaster1, 0, "VALUE = 0")
            gp.Con_sa(statisticsLevel1, OutRaster1, sumLevel1, "#", "VALUE >= 0")
            # calculating subnational totals for zones with difference >= 0
            gp.Con_sa(zones, outputs.resultLevel[level], OutRaster1, "#", "VALUE = 1")
            gp.Con_sa(OutRaster1, inputsClipped.cell_area_min, OutRaster2, "#", "VALUE > 0")
            gp.ZonalStatistics_sa(statisticsLevel1, "Value", OutRaster2, sumLevel2, "SUM", "DATA")
            # replacing 'NoData' values with zeros
            gp.IsNull_sa(sumLevel2, OutRaster2)
            gp.Con_sa(OutRaster2, sumLevel2, OutRaster1, 0, "VALUE = 0")
            gp.Con_sa(statisticsLevel1, OutRaster1, sumLevel2, "#", "VALUE >= 0")
            # calculating difference of the results of national and subnational analysis
            gp.Minus_sa(sumLevel2,sumLevel1,differ)
            # calculating national totals for zones with difference < 0
            gp.Con_sa(zones, outputs.combinedResult[level-1], OutRaster1, "#", "VALUE = -1")
            gp.Con_sa(OutRaster1, inputsClipped.cell_area_min, OutRaster2, "#", "VALUE > 0")
            gp.ZonalStatistics_sa(statisticsLevel1, "Value", OutRaster2, OutRaster3, "SUM", "DATA")
            # calculating calibrated values for areas where difference was < 0
            gp.Minus_sa(OutRaster3,differ,OutRaster1)
            # selecting only those areas, where difference was < 0# selecting only those areas, where difference was < 0
            gp.Con_sa(zones, OutRaster1, OutRaster2, "#", "VALUE = -1")
            # sometimes cropland area in remaining subnational units may result to negative
            # we select only positive numbers
            gp.Con_sa(OutRaster2, OutRaster2, OutRaster1, "#", "VALUE > 0")
            gp.Int_sa(OutRaster1, differ)
            gp.BuildRasterAttributeTable_management(differ, "OVERWRITE")
        
        #    cursor = gp.SearchCursor(differ)
        #    if cursor:
            # Processing calibrated raster:
            gui.PrintText('Processing calibrated areas for level ' + str(level))
            processLevel(paramsStruct, pathsAndUtilities, minMaxClass, differName,
                         outputs.resultForCalibratedLevel[level], gui)
        
            # combining final map
            gui.PrintText('Combining final map for level ' + str(level))
            gp.Con_sa(zones, outputs.combinedResult[level-1], OutRaster1, "#", "VALUE = 0")
            gp.Con_sa(zones, outputs.resultLevel[level], OutRaster2, OutRaster1, "VALUE = 1")
            gp.Con_sa(zones, outputs.resultForCalibratedLevel[level], outputs.combinedResult[level], OutRaster2, "VALUE = -1")
            gui.PrintText('Cleanup temporary rasters')
            gp.Delete_management(differ)
        else:
            gui.PrintText('Combining final map for level ' + str(level))
            gp.Con_sa(zones, outputs.combinedResult[level-1], OutRaster1, "#", "VALUE = 0")
            gp.Con_sa(zones, outputs.resultLevel[level], outputs.combinedResult[level], OutRaster1, "VALUE = 1")

    gp.Copy_management(outputs.combinedResult[level-1], outputs.combinedResult[level])
    
    pathsAndUtilities.cleanUp(tmp)
    #pathsAndUtilities.DeleteDir(tmpDir)