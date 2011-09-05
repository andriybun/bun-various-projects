def ReadDissimilarityMatrix(fileName):
    matrix = []    

    dmFile = open(fileName, 'r')
    rawDm = dmFile.readlines()
    for line in rawDm:
        row = []
        for val in line.split('\t'):
            row.append(int(val))
        matrix.append(row)
    
    return matrix
    
