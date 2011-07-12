import warnings

class map:
    
    def __init__(self):
        self.__data__ = []
        pass

    def __find__(self, key):
        # find value in a list of tuples with sorted first elements
        # return value - (element position, {0, 1} - element with a key value not exists/exists)
        lo = 0
        mid = lo
        hi = len(self.__data__)
        while lo < hi:
            mid = (lo + hi) // 2
            midkey = self.__data__[mid][0]
            if midkey < key:
                lo = mid+1
            elif midkey > key: 
                hi = mid
            else:
                return mid, True
        return hi, False

    def insert(self, key, val):
        elementPos, elementExists = self.__find__(key)
        if not elementExists:
            self.__data__.insert(elementPos, (key, val))
        else:
            warnings.warn('Element with a key "%s" already exists in a map! It will be replaced.' % str(key))
            self.__data__[elementPos] = (key, val)
        
    def find(self, key):
        elementPos, elementExists = self.__find__(key)
        if elementExists:
            return self.__data__[elementPos][1]
        else:
            warnings.warn('Element with a key "%s" not found!' % str(key))
            return 0

    def findIdx(self, key):
        elementPos, elementExists = self.__find__(key)
        if elementExists:
            return elementPos
        else:
            return -1

#if __name__ == "__main__":
#    mp = map()
#    mp.insert(1, 2)
#    mp.insert(2, 3)
#    mp.insert(3, 4)
#    mp.insert(2, 5)
#    print mp.find(2)
#    print mp.find(9)
#    print mp.findIdx(3)