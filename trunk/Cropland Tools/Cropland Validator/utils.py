# Some classes and functions used in script
from iterableStruct import iterableStruct
import os, arcgisscripting

#-------------------------------------------------------------------------
# utils class definition
#-------------------------------------------------------------------------
class utils:
    paramsStruct = iterableStruct()
    inputsNotClipped = iterableStruct()
    inputsClipped = iterableStruct()
    outputs = iterableStruct()
    tmp = iterableStruct()

    # constructor
    def __init__(self, gui, inputsNotClipped = None, output = None):
        # initialize arcgisscripting object
        self.gp = arcgisscripting.create()
        self.gp.CheckOutExtension("Spatial")
        self.gp.OverWriteOutput = 1 # allow overwriting rasters:
        # initialize roots and parameters
        self.paramsStruct = self.initParams()
        # initialize paths to all the rasters
        rasterNames = self.defineRasterPaths()
        if inputsNotClipped is None:
            self.inputsNotClipped = rasterNames.inputsNotClipped
        else:
            self.inputsNotClipped = inputsNotClipped
        self.inputsClipped = rasterNames.inputsClipped
        self.outputs = rasterNames.outputs
        if output is not None:
            self.outputs.resultStat = output
        self.tmp = rasterNames.tmp

    # define roots, parameters
    def initParams(self):
        paramsStruct = iterableStruct()
        
        paramsStruct.homeDir = os.getcwd() + "\\"   # current directory where the script is located
        paramsStruct.inputDir = "..\\input_new\\"   # move all the rasters and all data to this directory:
        paramsStruct.resultDir = "test_results_to_be_deleted\\" # result will be saved to this directory:
        paramsStruct.tmpDir = paramsStruct.homeDir + "tmp\\"
    
        # Reading configuration file
        if os.path.exists("settings.txt"):
            settings = open("settings.txt")
            i = 0
            while i < 4:
                line = settings.readline()
                while line[0] == '#':
                    line = settings.readline()
                if i == 0:
                    paramsStruct.inputDir = line.rstrip()
                elif i == 1:
                    paramsStruct.resultDir = line.rstrip()
                elif i == 2:
                    paramsStruct.minClass = int(line.rstrip())
                elif i == 3:
                    paramsStruct.maxClass = int(line.rstrip())
                i = i + 1;
    
        # Creating directories if necessary
        if not os.path.exists(paramsStruct.homeDir + paramsStruct.resultDir):
            os.mkdir(paramsStruct.homeDir + paramsStruct.resultDir)
        
        if not os.path.exists(paramsStruct.homeDir + "tmp\\"):
            os.mkdir(paramsStruct.homeDir + "tmp\\")
        
        return paramsStruct        

    # define all paths:
    def defineRasterPaths(self):
        paramsStruct = self.paramsStruct
        inputsNotClipped = iterableStruct()
        inputsClipped = iterableStruct()
        outputs = iterableStruct()
        tmp = iterableStruct()            
        
        #-------------------------------------------------------------------------------
        # Input rasters:
        #-------------------------------------------------------------------------------
        inputsNotClipped.mark_high_32     = paramsStruct.homeDir + paramsStruct.inputDir + "mark_high32"
        inputsNotClipped.countries        = paramsStruct.homeDir + paramsStruct.inputDir + "national"
        inputsNotClipped.subnationalUnits = paramsStruct.homeDir + paramsStruct.inputDir + "level1_img"
        inputsNotClipped.subregionalUnits = paramsStruct.homeDir + paramsStruct.inputDir + "level2"
        inputsNotClipped.cell_area        = paramsStruct.homeDir + paramsStruct.inputDir + "a_0083_1000"
        inputsNotClipped.statLayer        = paramsStruct.homeDir + paramsStruct.inputDir + "max_5maps-20n"
    
        inputsClipped.levelStatisticsName = []
        inputsClipped.levelStatisticsName.append("cl_level0") # countries
        inputsClipped.levelStatisticsName.append("cl_level1") # subnational units - regions
        inputsClipped.levelStatisticsName.append("cl_level2") # subregional units
        inputsClipped.mark_high_32name = "cl_mark_h32n"
        inputsClipped.mark_high_32     = paramsStruct.tmpDir + inputsClipped.mark_high_32name
        inputsClipped.countries        = paramsStruct.tmpDir + inputsClipped.levelStatisticsName[0]
        inputsClipped.subnationalUnits = paramsStruct.tmpDir + inputsClipped.levelStatisticsName[1]
        inputsClipped.subregionalUnits = paramsStruct.tmpDir + inputsClipped.levelStatisticsName[2]
        inputsClipped.cell_area        = paramsStruct.tmpDir + "cl_a_0083"
        inputsClipped.statLayer        = paramsStruct.tmpDir + "cl_mean"
    
        #-------------------------------------------------------------------------------
        # Resulting rasters:
        #-------------------------------------------------------------------------------
        # results of level analysis
        outputs.resultLevel = []
        outputs.resultForCalibratedLevel = ['dummy']
        outputs.combinedResult = ['dummy']
        outputs.resultLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resLevel_0")
        outputs.resultLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resLevel_1")
        outputs.resultLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resLevel_2")
        outputs.resultForCalibratedLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resCalibr_1")
        outputs.resultForCalibratedLevel.append(paramsStruct.homeDir + paramsStruct.resultDir + "resCalibr_2")
        outputs.combinedResult.append(paramsStruct.homeDir + paramsStruct.resultDir + "resCombi_1")
        outputs.combinedResult.append(paramsStruct.homeDir + paramsStruct.resultDir + "resCombi_2")
        outputs.resultStat = outputs.combinedResult[2] + "st"
        outputs.result_area = paramsStruct.homeDir + paramsStruct.resultDir + "result_ar"
        outputs.result_sum = paramsStruct.homeDir + paramsStruct.resultDir + "result_sm"
    
        #-------------------------------------------------------------------------------
        # Temporary rasters:
        #-------------------------------------------------------------------------------
        tmp.closest       = paramsStruct.tmpDir + "cl_close"
        tmp.combined      = paramsStruct.tmpDir + "combine"
        tmp.cell_area_min = paramsStruct.tmpDir + "cl_a_min"
        tmp.OutRaster1    = paramsStruct.tmpDir + "tmp1"
        tmp.OutRaster2    = paramsStruct.tmpDir + "tmp2"
        tmp.OutRaster3     = paramsStruct.tmpDir + "tmp3"
        tmp.OutClass       = paramsStruct.tmpDir + "class"
        tmp.differName = "differ"
        tmp.differ = paramsStruct.tmpDir + tmp.differName
        tmp.zones = paramsStruct.tmpDir + "zones"
        tmp.areaUnits = paramsStruct.tmpDir + "unit"
        tmp.sumLevel1 = paramsStruct.tmpDir + "sum_l1"
        tmp.sumLevel2 = paramsStruct.tmpDir + "sum_l2"

    
        output = iterableStruct()
        output.inputsNotClipped = inputsNotClipped
        output.inputsClipped = inputsClipped
        output.outputs = outputs
        output.tmp = tmp
        
        return output

    # verify if rasters exist
    def verifyRasters(self, listOfRasters, gui):
        for rasterName in listOfRasters:
            if (not self.gp.Exists(rasterName)):
                gui.PrintText('Error: Raster does not exist: ' + rasterName);
                return 1
#                raise IOError('Raster does not exist: ' + rasterName)

    # clip an area from input raster and convert result to integer
    def clipRasterInt(self, inputRaster, outputRaster, coords):
        OutRaster1 = self.tmp.OutRaster1
        if coords is None or coords == '#':
            self.gp.copy_management(inputRaster, OutRaster1)
        else:
            self.gp.clip_management(inputRaster, coords, OutRaster1)
        self.gp.Int_sa(OutRaster1, outputRaster)

    # clip an area from input raster    
    def clipRaster(self, inputRaster, outputRaster, coords):
        if coords is None or coords == '#':
            self.gp.copy_management(inputRaster, outputRaster)
        else:
            self.gp.clip_management(inputRaster, coords, outputRaster)

    # converting inputs to proper units
    def prepareRasters(self):
        inputsClipped = self.inputsClipped
        tmp = self.tmp
        self.gp.Float_sa(inputsClipped.cell_area, tmp.OutRaster2)
        self.gp.Divide_sa(tmp.OutRaster2, 10, inputsClipped.cell_area)
        self.gp.Float_sa(inputsClipped.statLayer, tmp.OutRaster2)
        self.gp.Divide_sa(tmp.OutRaster2, 100, tmp.OutRaster1)
        self.gp.Times_sa(tmp.OutRaster1, inputsClipped.cell_area, tmp.cell_area_min)
        
    # processing results
    def processResults(self):
        self.gp.Con_sa(self.outputs.combinedResult[2], self.inputsClipped.statLayer, 
                       self.outputs.resultStat, "#", "VALUE > 0")
                       
    # delete temp rasters
    def cleanUp(self, list):
        for rasterName in list:
            if (self.gp.Exists(rasterName)):
                self.gp.delete_management(rasterName)