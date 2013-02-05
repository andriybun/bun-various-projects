import csv
from rasterAgreementTable import rasterAgreementTable

def parseCsv(csvFileName, csvFileNameOut):
    csvFile = open(csvFileName, 'rb')
    priorityDataCsv = csv.reader(csvFile, delimiter=',', quotechar='"')
    csvFileOut = open(csvFileNameOut, 'wb')
    csvOutWriter = csv.writer(csvFileOut, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    
#    priorityDataCsv = csv.reader(csvFileName, delimiter=',', quotechar='"')
#    csvOutWriter = csv.writer(csvFileNameOut, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    
    rowIdx = 1
    for row in priorityDataCsv:
        if rowIdx == 1:
            numRasters = len(row) - 2
            header = ['Contry ID'];
            headerPrior1 = ['Priority data 1']
            headerPrior2 = ['Priority data 2']
            headerWeights = ['Weights']
            for i in range(numRasters - 1):
                headerPrior1.append('')
                headerPrior2.append('')
                headerWeights.append('')
            header.extend(headerPrior1)
            header.extend(headerPrior2)
            header.extend(headerWeights)
            csvOutWriter.writerow(header)
        if rowIdx > 1:
            if len(row) != numRasters + 2:
                raise Exception('Wrong csv file format')
            
            priorList = []
            for idx in range(2, numRasters+2):
                priorList.append(float(row[idx]))
            
            rat = rasterAgreementTable(priorList)
            
            zoneId = int(row[0])
            rowOut = [str(zoneId)]
            rowOut[1:numRasters] = rat.priorityValues1
            rowOut[1*numRasters+1 : numRasters*2] = rat.priorityValues2
            rowOut[2*numRasters+1 : numRasters*3] = rat.weights
            csvOutWriter.writerow(rowOut)
    
        rowIdx += 1
        
        
if __name__ == '__main__':
    csvFile = open('M:/Andriy/new_run/Results_Global/120826_test/res.csv', 'rb')
    csvFileOut = open('M:/Andriy/new_run/Results_Global/120826_test/prior.csv', 'wb')
    parseCsv(csvFile, csvFileOut)