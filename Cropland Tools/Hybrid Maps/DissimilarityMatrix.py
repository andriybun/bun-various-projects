def ReadDissimilarityMatrix(fileName):
    matrix = []    

    dmFile = open(fileName, 'r')
    rawDm = dmFile.readlines()
    for line in rawDm:
        row = []
        for val in line.split('\t'):
            row.append(int(val))
        matrix.append(row)
    
    # copy upper right matrix part to lower left to make it symmetric
    matrixSize = len(matrix)
    for idx1 in range(0, matrixSize):
        if not(len(matrix[idx1]) == matrixSize):
            raise Exception('Dissimilarity matrix dimensions must agree')
        for idx2 in range(idx1+1, matrixSize):
            matrix[idx2][idx1] = matrix[idx1][idx2]
    
    return matrix
    
