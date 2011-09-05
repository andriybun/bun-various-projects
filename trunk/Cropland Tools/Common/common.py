import math

def MakeListString(inList):
    listString = ""
    for name in inList:
        listString += "\'" + name + "\';"
    listString = listString[0:-1]
    return listString
    
def CompareEq(val1, val2, precision):
    return (math.fabs(val1 - val2) < precision)
    
def CompareNe(val1, val2, precision):
    return not(CompareEq(val1, val2, precision))