# -*- coding: utf-8 -*-
"""
Created on Wed Dec 08 22:16:02 2010

@author: Andriy Bun
@name:   rasterAgreementTable
"""

from utils import *

def invertPriorities(priorityVector):
    maxVal = max(priorityVector) + 1
    res = []
    for val in priorityVector:
        res.append(maxVal - val)
    return res  

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
        self.SetResultingClasses()

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
                
            self.data.append(dict(resultingClass = -1, \
                classes1 = self.classes1[i], \
                classes2 = self.classes2[i], \
                agreementTable = self.agreementTable[i] \
            ))

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

    def FindClass(self, val1, val2):
        i = 0
        while (self.data[i]['classes1'] != val1) or (self.data[i]['classes2'] != val2):
            i = i + 1
        return self.data[i]['resultingClass']

    #===============================================================================
    # Bubble sort
    #===============================================================================
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
    # Set resulting classes 
    #===============================================================================
    def SetResultingClasses(self):
        currentClass = 1
        numRecords = len(self.data)
        self.data[0]['resultingClass'] = currentClass
        for idx in range(1, numRecords):
            if self.data[idx]['classes1'] != self.data[idx-1]['classes1'] and \
               self.data[idx]['classes2'] != self.data[idx-1]['classes2']:
                currentClass += 1
            self.data[idx]['resultingClass'] = currentClass

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
        header0 = '%-12s|       |       || ' % (' ')
        header1 = '%-12s| SumW1 | SumW2 ||  Rasters' % (' Cell class')
        header2 = '%-12s|       |       || ' % (' ')
        for i in range(0, self.numRasters):
            header2 += '%2d  | ' % (i + 1)
        header2 = header2[0:-2]
        if gui is None:
            print '\n'
            print tableCaption
            print headerSeparator
            print header0
            print header1
            print header2
            print headerSeparator
        else:
            gui.PrintText('\n')
            gui.PrintText(tableCaption)
            gui.PrintText(headerSeparator)
            gui.PrintText(header0)
            gui.PrintText(header1)
            gui.PrintText(header2)
            gui.PrintText(headerSeparator)
        for row in self.data:
            outString = '      %4d  |  %3d  |  %3d  ||  ' % (i, row['classes1'], row['classes2'])
            for val in row['agreementTable']:
                outString += str(val) + '  |  '
            outString = outString[0:-3]
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

    #===============================================================================
    # Print table
    #===============================================================================
    def PrintToFile(self, file, listOfRasters = None):
        import os
        file.write('Priority values 1:\n')
        file.write(str(invertPriorities(self.priorityValues1)) + '\n')
        file.write('Priority values 2:\n')
        file.write(str(invertPriorities(self.priorityValues2)) + '\n')
        i = 0;
        tableCaption = 'Raster agreement table\n'
        headerSeparator = '-------------------------------'
        for val in self.data[0]['agreementTable']:
            headerSeparator += '------'
        header0 = '%-12s|       |       ||  Rasters' % (' ')
        header1 = '%-12s| SumW1 | SumW2 || ' % (' Cell class')
        header2 = '%-12s|       |       || ' % (' ')
        for i in range(0, self.numRasters):
            header2 += '%2d  | ' % (i + 1)
        header2 = header2[0:-2]
        file.write('\n')
        file.write(tableCaption + '\n')
        file.write(headerSeparator + '\n')
        file.write(header0 + '\n')
        file.write(header1 + '\n')
        file.write(header2 + '\n')
        file.write(headerSeparator + '\n')

        for row in self.data:
            outString = '      %4d  |  %3d  |  %3d  ||  ' % (i, row['classes1'], row['classes2'])
            for val in row['agreementTable']:
                outString += str(val) + '  |  '
            outString = outString[0:-3]
            i = i + 1
            file.write(outString + '\n')
        file.write(headerSeparator + '\n')
        file.write('\n\n')
        if not(listOfRasters is None):
            file.write('Rasters:\n')
            i = 1
            for rasterName in listOfRasters:
                legend = '%d. - %s' % (i, os.path.basename(rasterName))
                i += 1
                file.write(legend + '\n')

if __name__ == "__main__":
    RAT = rasterAgreementTable([2, 1, 1, 1, 1])
    RAT.Print()
    print str(RAT.FindFirst(8, 4))