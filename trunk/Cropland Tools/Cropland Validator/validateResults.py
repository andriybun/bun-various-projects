# -*- coding: utf-8 -*-
"""
Created on Fri May 13 09:57:26 2011

author: Andriy Bun
name:   Validator of cropland

"""

import os
import sys
import arcgisscripting

def ValidateRaster(rasterToValidate, areaGrid, nationalStatistics):
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1

    TMPDIR = os.path.dirname(rasterToValidate) + "\\validation_tmp\\"
    if not os.path.isdir(TMPDIR):
        os.mkdir(TMPDIR)
    tmpRaster = TMPDIR + "tmpRaster.img"
    tmpCombined = TMPDIR + "tmpCombined.img"
    nationalStatisticsInt = TMPDIR + "natStatInt.img"

    gp.Times_sa(rasterToValidate, areaGrid, tmpRaster)
    gp.Divide_sa(tmpRaster, 1000, tmpCombined)
    gp.Int_sa(nationalStatistics, nationalStatisticsInt)
    gp.ZonalStatistics_sa(nationalStatisticsInt, "Value", tmpCombined, tmpRaster, "SUM", "DATA")
    gp.Combine_sa("'" + nationalStatisticsInt + "';'" + tmpRaster + "'", tmpCombined)

    nationalStatisticsName = os.path.splitext(os.path.basename(nationalStatisticsInt))[0]
    tmpRasterName = os.path.splitext(os.path.basename(tmpRaster))[0]

    gp.AddMessage("\nnat. stat.\t|  computed\t|  error")
    gp.AddMessage("---------------------------------------")

    rows = gp.searchcursor(tmpCombined)
    row = rows.next()
    while row:
        natStat = row.getValue(nationalStatisticsName)
        computed = row.getValue(tmpRasterName)
        diff = (float(computed) - natStat) / computed * 100
        outString = "%d\t|  %d\t|  %5.2f" % (natStat, computed, diff)
        gp.AddMessage(outString)
        row = rows.next()

#    gp.delete_management(tmpRaster)
#    gp.delete_management(tmpCombined)
#    gp.delete_management(nationalStatisticsInt)
    os.removedirs(TMPDIR)

if __name__ == '__main__':
    rasterToValidate = sys.argv[1]
    areaGrid = sys.argv[2]
    nationalStatistics = sys.argv[3]
    ValidateRaster(rasterToValidate, areaGrid, nationalStatistics)
