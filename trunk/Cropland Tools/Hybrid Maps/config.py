import os
import shutil
import arcgisscripting
from iterableStruct import iterableStruct

class config():
    
    def __init__(self, gui, paths):
        self.gui = gui
        self.gp = arcgisscripting.create()
        self.gp.CheckOutExtension("Spatial")
        self.gp.OverWriteOutput = 1
        self.__InitPaths__(paths)
        self.num_rasters = len(paths.input.landMaps)
        
        gui.PrintTextTime("Verifying")
        self.VerifyRasters(self.paths.input, gui)

    def __InitPaths__(self, paths):
        self.paths = paths
        self.paths.tmp = iterableStruct()
        self.paths.tmp.dir = os.getcwd() + '\\tmp\\'
        if not os.path.isdir(self.paths.tmp.dir):
            os.mkdir(self.paths.tmp.dir)

    # Verify if rasters exist
    def VerifyRasters(self, ListOfRasters, gui):
        flag = True
        for RasterName in ListOfRasters:
            if type(RasterName) == list:
                self.VerifyRasters(RasterName, gui)
            else:
                if (not self.gp.Exists(RasterName)):
                    gui.Error('Raster does not exist: ' + RasterName)
                    raise Exception('Raster validation', 'Raster does not exist: ' + RasterName)
                    flag = False
        return flag

    # Delete all rasters in a list
    def DeleteRasters(self, ListOfRasters):
        for RasterName in ListOfRasters:
            if type(RasterName) == list:
                self.DeleteRasters(RasterName)
            else:
                self.gp.delete_management(RasterName)

