# -*- coding: utf-8 -*-
"""
Created on Wed Dec 08 22:16:02 2010

@author: Andriy Bun
@name:   rasterAgreementTable
"""

from utils import *

class rasterAgreementTable():
    
    def __init__(self, priorityValues1, priorityValues2 = []):

        self.priorityValues1 = priorityValues1
        self.priorityValues2 = priorityValues2

        if len(priorityValues2) == 0:
            self.priorityValues2 = []
            for i in range(0, len(priorityValues1)):
                self.priorityValues2.append(1)

        # Verify inputs
        if len(priorityValues1) != len(priorityValues1):
            raise Exception('Priority vectors must have the same size!')
            
        self.numRasters = len(self.priorityValues1)
        self.weights = self.GetWeights(self.priorityValues1)
        self.GetClasses()
        self.SortTable(['classes1', 'classes2'])

    #===============================================================================
    # Method to parse rasters' priorities to weights
    #===============================================================================
    def GetWeights(self, priorityValues):
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
    # Get weight classes recursive
    #===============================================================================
    def GetClasses(self):
        self.classes1 = []
        self.classes2 = []
        self.agreementTable = []
        self.data = []
        self.curRow = 0

        for i in range(0, 2**self.numRasters):
            self.agreementTable.append([])

        self.GenerateAgreementTable(self.weights, 0)

        for i in range(0, 2**self.numRasters):
            self.classes1.append(0)
            self.classes2.append(0)
           
            for j in range(0, self.numRasters):
                self.classes1[i] += self.agreementTable[i][j] * self.weights[j]
                self.classes2[i] += self.agreementTable[i][j] * self.priorityValues2[j]
                
            self.data.append(dict(classes1 = self.classes1[i], classes2 = self.classes2[i], agreementTable = self.agreementTable[i]))

        self.classes1.sort()
    
    # Recursive
    def GenerateAgreementTable(self, weights, weightsSum):
        if len(weights) > 1:
            self.agreementTable[self.curRow].append(1)
            self.GenerateAgreementTable(weights[1:len(weights)], weightsSum + weights[0])
            for i in self.agreementTable[self.curRow-1][0:-len(weights)]:
                self.agreementTable[self.curRow].append(i)
            self.agreementTable[self.curRow].append(0)
            self.GenerateAgreementTable(weights[1:len(weights)], weightsSum)
        else:
            #self.classes1.append(weightsSum + weights[0])
            #self.classes1.append(weightsSum)
            for i in self.agreementTable[self.curRow]:
                self.agreementTable[self.curRow+1].append(i)
            self.agreementTable[self.curRow].append(1)
            self.agreementTable[self.curRow+1].append(0)
            self.curRow += 2
    
    #===============================================================================
    # Sort table by multiple fields
    #===============================================================================
    def SortTable(self, fields):
        fields.reverse()
        for field in fields:
            self.data = self.BubbleSort(self.data, field)

    #===============================================================================
    # Find
    #===============================================================================
    def FindFirst(self, val1, val2):
        i = 0
        while (self.data[i]['classes1'] != val1) or (self.data[i]['classes2'] != val2):
            i = i + 1
        return i

    def BubbleSort(self, tab, field):
        i = 0
        while (i < len(tab) - 1):
            flag = 0
            while (tab[i][field] > tab[i+1][field]):
                tmp = tab[i]
                tab[i] = tab[i+1]
                tab[i+1] = tmp
                flag = 1
            if (flag):
                i = max(i - 1, 0)
            else:
                i = i + 1
        return tab
        
    #===============================================================================
    # Print table
    #===============================================================================
    def Print(self, gui = None, listOfRasters = None):
        import os
        i = 0;
        tableCaption = 'Raster agreement table'
        headerSeparator = '-------------------------------'
        for val in self.data[0]['agreementTable']:
            headerSeparator += '------'
        header = '%-12s| SumW1 | SumW2 |  Rasters' % (' Cell class')
        if gui is None:
            print '\n'
            print tableCaption
            print headerSeparator
            print header
            print headerSeparator
        else:
            gui.PrintText('\n')
            gui.PrintText(tableCaption)
            gui.PrintText(headerSeparator)
            gui.PrintText(header)
            gui.PrintText(headerSeparator)
        for row in self.data:
            outString = '      %4d  |  %3d  |  %3d' % (i, row['classes1'], row['classes2'])
            for val in row['agreementTable']:
                outString += '  |  ' + str(val)
            i = i + 1
            if gui is None:
                print outString
            else:
                gui.PrintText(outString)
        if gui is None:
            print headerSeparator
            print '\n'
        else:
            gui.PrintText(headerSeparator)
            gui.PrintText('\n')
        if not(listOfRasters is None):
            if gui is None:
                print 'Rasters:'
            else:
                gui.PrintText('Rasters:')
            i = 1
            for rasterName in listOfRasters:
                legend = '%d. - %s' % (i, os.path.basename(rasterName))
                i += 1
                if gui is None:
                    print legend
                else:
                    gui.PrintText(legend)