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
        self.paths.extension = '.img'
        self.paths.tmp = iterableStruct()
        self.paths.tmp.dir = os.getcwd() + '\\tmp_%s\\' % (os.getenv('COMPUTERNAME'))
        if os.path.isdir(self.paths.tmp.dir):
            self.DeleteDir(self.paths.tmp.dir)
        os.mkdir(self.paths.tmp.dir)
        self.paths.tmp.combinedRaster = self.paths.tmp.dir + 'combined' + self.paths.extension
        self.paths.tmp.singleMapNameTemplate = 'map_%d'

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

    # Delete all files from a directory
    def DeleteDir(self, dir):
        if os.path.exists(dir):
            shutil.rmtree(dir)

    def ClipSingleRaster(self, inRaster, outRaster, coords = None):
        if coords is None or coords == '#':
            self.gp.copy_management(inRaster, outRaster)
        else:
            self.gp.clip_management(inRaster, coords, outRaster)
        

    def ClipRasters(self, inRasters, destinationFolder, clippedNameTemplate = None, coords = None):
        clippedLayerList = []
        rasterNum = 1
        if type(inRasters) == list:
            for raster in inRasters:
                if type(raster) == list:
                    if clippedNameTemplate is not None:
                        self.gui.Error('List of rasters contains inner lists - name template cannot be applied')
                    else:
                        self.ClipRasters(raster, destinationFolder, coords = coords)
                if clippedNameTemplate is None:
                    rasterName = os.path.basename(raster)
                    clippedName = destinationFolder + rasterName
                else:
                    clippedName = destinationFolder + clippedNameTemplate % rasterNum
                clippedLayerList.append(clippedName)
                self.ClipSingleRaster(raster, clippedName, coords)
                rasterNum = rasterNum + 1
        else:
            rasterName = os.path.basename(raster)
            clippedName = destinationFolder + rasterName
            self.ClipSingleRaster(inRasters, clippedName, coords)
            
        return clippedLayerList