

def MakeListString(inList):
    listString = ""
    for name in inList:
        listString += "\'" + name + "\';"
    listString = listString[0:-1]
    return listString