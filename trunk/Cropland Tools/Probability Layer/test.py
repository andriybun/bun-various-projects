from rasterAgreementTable import rasterAgreementTable

## Debug
#lst  = [1, 2, 3, 4]
#wgh = GetWeights(lst)
#print wgh
#cls = GetClasses(wgh)
#print cls
#print findFirst(cls, 1)
#print findFirst(cls, 2)
#print findFirst(cls, 3)
#print findFirst(cls, 4)
tb = rasterAgreementTable([1, 1, 1, 1, 1], [1, 1, 1, 1, 1])
print 'Weights 1st type: ' + str(tb.weights)
print 'Weights 2nd type: ' + str(tb.priorityValues2)
tb.Print()

#print '====='
#tb.SortTable(['classes1', 'classes2'])
#tb.Print()

#print tb.FindFirst(0, 0)
#print tb.FindFirst(1, 1)
#print tb.FindFirst(1, 1)
#print tb.FindFirst(1, 2)
#print tb.FindFirst(2, 2)
#print tb.FindFirst(2, 3)
#print tb.FindFirst(2, 3)
#print tb.FindFirst(3, 4)
#print tb.FindFirst(4, 1)
#print tb.FindFirst(5, 2)
#print tb.FindFirst(5, 2)
#print tb.FindFirst(5, 3)
#print tb.FindFirst(6, 3)
#print tb.FindFirst(6, 4)
#print tb.FindFirst(6, 4)
#print tb.FindFirst(7, 5)