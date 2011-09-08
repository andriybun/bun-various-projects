import sys
from sets import Set

from DissimilarityMatrix import ReadDissimilarityMatrix

def computeBestClass(classesInCell, dissimilarityMatrix):
    numClasses = len(dissimilarityMatrix)
    bestDissimilarity = [sys.maxint]
    bestClasses = Set([-1]);
    idx1 = 0
    for class1 in classesInCell:
        dissimilarity = 0
        idx2 = 0
        for class2 in classesInCell:
            if class1 < 1 or class1 > numClasses:
                raise Exception('Input map #%d contains cropland class %d which does not exist in dissimilarity matrix' % (idx1+1, class1))
            elif class2 < 1 or class2 > numClasses:
                raise Exception('Input map #%d contains cropland class %d which does not exist in dissimilarity matrix' % (idx2+1, class2))
            else:
                dissimilarity = dissimilarity + dissimilarityMatrix[class1-1][class2-1]
            idx2 = idx2 + 1
        if dissimilarity < bestDissimilarity[0]:
            bestDissimilarity = [dissimilarity]
            bestClasses = Set([classesInCell[idx1]])
        elif dissimilarity == bestDissimilarity[0]:
            bestDissimilarity.append(dissimilarity)
            bestClasses.add(classesInCell[idx1])
        idx1 = idx1 + 1
    bestClassesList = []
    for cl in bestClasses:
        bestClassesList.append(cl)
    return bestClassesList

# Test1
if __name__ == '__main__':
    dissimilarityMatrixFile = 'dissimilarity_matrix.txt'
    dissimilarityMatrix = ReadDissimilarityMatrix(dissimilarityMatrixFile)
    classesInCell = [1, 4, 9, 3, 9]
    bestClasses = computeBestClass(classesInCell, dissimilarityMatrix)
    print 'best classes:'
    print '\t' + str(bestClasses)
