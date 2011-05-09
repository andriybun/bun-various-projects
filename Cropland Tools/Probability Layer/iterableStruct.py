class iterableStruct(object):
    def __iter__(self):
        for item in self.__dict__:
            yield self.__dict__[item]