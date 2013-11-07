# -*- coding: utf-8 -*-
"""
Created on Mon Nov 29 15:36:20 2010

@author: Andriy Bun

@name:   Useful functions
"""

import math
import os

# Floating point operations tolerance
EPSILON = 1e-6

def AddSuffixToName(name, suffix):
    return os.path.splitext(name)[0] + suffix + os.path.splitext(name)[1]

#===============================================================================
#  Method to replace all values in a raster with 'weight', and zeros or noData
#  with 0
#===============================================================================
def MakeRasterOfValues(gp, rast, order, weight, weight2, out):
    onesRast = AddSuffixToName(out, "_count")
    gp.Con_sa(rast, 1, onesRast, 0, "VALUE > 1e-6")
    gp.BuildRasterAttributeTable_management(onesRast, "OVERWRITE")
    gp.Times_sa(onesRast, order, out)
    gp.Times_sa(onesRast, weight, AddSuffixToName(out, "_one"))
    gp.Times_sa(onesRast, weight2, AddSuffixToName(out, "_two"))

#==============================================================================
# Ceck a list of rasters for having the same extent
#==============================================================================
def GetRasterExtent(gp, raster):
    propertyNames = ['TOP', \
                     'BOTTOM', \
                     'LEFT', \
                     'RIGHT', \
                     'CELLSIZEX', \
                     'CELLSIZEY']
    numProperties = len(propertyNames);
    extent = [0] * numProperties

    for i in range(numProperties):
        extent[i] = gp.GetRasterProperties_management(raster, propertyNames[i])
    
    return extent
    
def IsSameExtent(gp, rasterList):
    firstRaterExtent = GetRasterExtent(gp, rasterList[1])
    for raster in rasterList[1:]:
        if not (max(abs(firstRaterExtent - GetRasterExtent(gp, raster))) > EPSILON):
            return False
    return True

#===============================================================================
# Validation function
#===============================================================================
def Validate(interface, runConfig, priorityValues = None, priorityValues2 = None):
    interface.PrintText('Validating result...')
    import arcgisscripting
    from rasterAgreementTable import rasterAgreementTable
    inputs = runConfig.paths.inputs
    result = runConfig.paths.result
    gp = arcgisscripting.create()
    gp.CheckOutExtension("Spatial")
    gp.OverWriteOutput = 1
    
    inputList = inputs.LayerList
    
    if priorityValues is None:
        priorityValues = []
        for i in range(0, runConfig.num_rasters):
            priorityValues.append(1)
    
    if priorityValues2 is None:
        priorityValues2 = []
        for i in range(0, runConfig.num_rasters):
            priorityValues2.append(1)
    
    agreementTable = rasterAgreementTable(priorityValues, priorityValues2)
    
#    weights = GetWeights(priorityValues)

    # Land classes:
#    landClasses = GetClasses(weights)

#    interface.PrintText(str(weights))
#    interface.PrintText(str(landClasses))

    xMin = gp.GetRasterProperties(result, 'LEFT')
    yMin = gp.GetRasterProperties(result, 'BOTTOM')
    xMax = gp.GetRasterProperties(result, 'RIGHT')
    yMax = gp.GetRasterProperties(result, 'TOP')
    
    print xMin
    print xMax
    print yMin
    print yMax
    
    xCount = gp.GetRasterProperties(result, 'COLUMNCOUNT')
    yCount = gp.GetRasterProperties(result, 'ROWCOUNT')
    
    xCellSize = gp.GetRasterProperties(result, 'CELLSIZEX')
    yCellSize = gp.GetRasterProperties(result, 'CELLSIZEY')
    
    xRes = 100
    yRes = 100
    
#    print xRes
#    print yRes
    
    xStep = xCellSize * math.floor(xCount / xRes)
    yStep = yCellSize * math.floor(yCount / yRes)
    
    print xStep
    print yStep
    
    xGrid = []
    yGrid = []
    
    for i in xrange(xRes):
        xGrid.append(xMin + xCellSize / 2 + xStep * (i+1))

    for i in xrange(yRes):
        yGrid.append(yMin + yCellSize / 2 + yStep * (i+1))

    numPoints = 0

    for x in xGrid:
        for y in yGrid:
            coord = str(x) + ' ' + str(y)
            if (gp.GetCellValue_management(result, coord) == 'NoData') or (float(gp.GetCellValue_management(result, coord)) == 0):
                break
            inputValues = []
            rastersAgree = 0
            idx = 0
            for raster in inputList:
                val = gp.GetCellValue_management(raster, coord)
                inputValues.append(val)
                if not (float(val) == 0):
                    rastersAgree = rastersAgree + agreementTable.weights[idx]
                idx = idx + 1
            resultValue = gp.GetCellValue_management(result, coord)
            if resultValue =='NoData':
                resultValue = 0
#            interface.PrintText(str(resultValue))
#            interface.PrintText(str(inputValues))
#            interface.PrintText(str(agreementTable.data[int(resultValue)]['classes1']))
#            interface.PrintText(str(rastersAgree))
#            interface.PrintText(str(gp.GetCellValue_management(inputs.units, coord)))
#            interface.PrintText('=====')
            if not (rastersAgree == agreementTable.data[int(resultValue)]['classes1']):
                interface.PrintText('Point: (' + str(x) + ', ' + str(y) + '): Expected rasters agree - ' + str(rastersAgree) + ', Computed - ' + str(agreementTable.data[int(resultValue)]['classes1']))
                interface.Error('Result validation failed')
            numPoints = numPoints + 1
    interface.PrintText('Result validation completed successfully on ' + str(numPoints) + ' point(s)')

#===============================================================================
# Find a value in a sorted list
#===============================================================================
def findFirst(lst, val):
    lo = 0
    hi = len(lst)
    while lo < hi:
        mid = (lo + hi) // 2
        midval = lst[mid]
        if midval < val:
            lo = mid+1
        elif midval > val: 
            hi = mid
        else:
            while mid > 0 and lst[mid-1] == val:
                mid = mid - 1
            return mid
    return -1
"""
#===============================================================================
# Method to parse rasters' priorities to weights
#===============================================================================
def GetWeights(priorityValues):
    priorityValuesSorted = priorityValues[:]
    priorityValuesSorted.sort()
    numValues = len(priorityValues)
    priorityCumSum = [1]
    weights = []
    idx = 0
    for i in range(1, numValues):
        priorityCumSum.append(priorityCumSum[i-1] + priorityCumSum[idx])
        if not(priorityValuesSorted[i] == priorityValuesSorted[i-1]):
            idx = i
    # Put weights in proper order
    for val in priorityValues:
        weights.append(max(1, priorityCumSum[findFirst(priorityValuesSorted, val)]))

#    # Debug
#    print priorityValues
#    print priorityValuesSorted
#    print priorityCumSum
#    print weights
    return weights


#===============================================================================
# Another handy function
#===============================================================================
global classes
global agreementTable
global curRow

#===============================================================================
# TODO: Process table
#===============================================================================

def GetWeightSumClasses(weights, weightsSum):
    global classes
    global agreementTable
    global curRow
    if len(weights) > 1:
        agreementTable[curRow].append(1)
        GetWeightSumClasses(weights[1:len(weights)], weightsSum + weights[0])
        for i in agreementTable[curRow-1][0:-len(weights)]:
            agreementTable[curRow].append(i)
        agreementTable[curRow].append(0)
        GetWeightSumClasses(weights[1:len(weights)], weightsSum)
    else:
        classes.append(weightsSum + weights[0])
        classes.append(weightsSum)
        for i in agreementTable[curRow]:
            agreementTable[curRow+1].append(i)
        agreementTable[curRow].append(1)
        agreementTable[curRow+1].append(0)
        curRow += 2
    return classes

def GetClasses(weights):
    global classes
    global agreementTable
    global curRow
    classes = []
    agreementTable = []
    curRow = 0
    print '===='
    for i in range(0, 2**len(weights)):
        agreementTable.append([])
    classes = GetWeightSumClasses(weights, 0)
    tab = []
    for i in range(0, 2**len(weights)):
        tab.append(dict(classes = classes[i], agreementTable = agreementTable[i]))
        #print agreementTable[i]
        print tab[i]
    classes.sort()
    tab.sort()
    for i in range(0, 2**len(weights)):
        print tab[i]
    return classes
"""