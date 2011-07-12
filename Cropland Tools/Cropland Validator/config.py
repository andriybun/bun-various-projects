# -*- coding: utf-8 -*-
"""
Created on Mon Nov 29 11:36:34 2010

@author: Andriy Bun

@name:   Useful utils
"""

import os
import shutil
import arcgisscripting
from iterableStruct import iterableStruct

class config():
    
    num_rasters = 4
    
    paths = iterableStruct()

    def __init__(self, gui, inputPaths, coords, doClip = 1):
        self.gui = gui
        self.gp = arcgisscripting.create()
        self.gp.CheckOutExtension("Spatial")
        self.gp.OverWriteOutput = 1
        self.__InitPaths__(inputPaths)
        self.num_rasters = len(self.paths.inputs.LayerList)
        gui.PrintTextTime("Verifying")
        self.VerifyRasters(self.paths.inputs, gui)
        gui.PrintTextTime("Clipping inputs")
        if doClip == 1:
            self.ClipRasters(coords)

    def __InitPaths__(self, inputPaths = None):
        # Main directories
        self.paths.HOMEDIR = os.getcwd() + "\\"
        self.paths.INPUTDIR = self.paths.HOMEDIR + "input\\"
        self.paths.RESULTDIR = self.paths.HOMEDIR + "results\\"
        self.paths.TMPDIR = self.paths.HOMEDIR + "tmp\\"
        
        self.DeleteDir(self.paths.TMPDIR)
        if not os.path.isdir(self.paths.TMPDIR):
            os.mkdir(self.paths.TMPDIR)
        
        if inputPaths is None:
            pass
## TODO: add here
#            # Input rasters:
#            self.paths.inputs = iterableStruct()
#            
#            inputLayerPrefix = self.paths.INPUTDIR + "layer"
#            self.paths.inputs.units            = self.paths.INPUTDIR + "countries"
#            self.paths.inputs.cellAreas        = self.paths.INPUTDIR + "cell_areas"
#            
#            self.paths.inputs.LayerList = []
#            
#            for i in range(1, self.num_rasters + 1):
#                self.paths.inputs.LayerList.append(inputLayerPrefix + str(i))
#    
#            # Result:
#            if not os.path.isdir(self.paths.RESULTDIR):
#                os.mkdir(self.paths.RESULTDIR)
#
#            self.paths.result = self.paths.RESULTDIR + "mark_high"
#            self.paths.resultAvg = self.paths.RESULTDIR + "mark_high_a"
        else:
            # Input rasters:
            self.paths.inputs = iterableStruct()
            
            self.paths.inputs.units     = inputPaths.countries
            self.paths.inputs.cellAreas = inputPaths.cellAreas
            self.paths.inputs.LayerList = inputPaths.croplandLayerList
            
            # Result:
            self.paths.result = inputPaths.result
            self.paths.resultAvg = inputPaths.resultAvg
        
        # Temporary rasters:
        self.paths.tmp = iterableStruct()

        tmpPrefix = self.paths.TMPDIR + "tmp"
        
        self.paths.tmp.LayerList = []
        
        for i in range(1, len(self.paths.inputs.LayerList) + 1):
            self.paths.tmp.LayerList.append(tmpPrefix + str(i))

        self.paths.tmp.temp1      = self.paths.TMPDIR + "temp1"
        self.paths.tmp.temp2      = self.paths.TMPDIR + "temp2"
        self.paths.tmp.temp3      = self.paths.TMPDIR + "temp3"
        self.paths.tmp.temp4      = self.paths.TMPDIR + "temp4"
        self.paths.tmp.sumRast    = self.paths.TMPDIR + "sumRast"
        self.paths.tmp.sumRastOne = self.paths.TMPDIR + "sumRastOne"
        self.paths.tmp.sumRastTwo = self.paths.TMPDIR + "sumRastTwo"

    # clip an area from input raster    
    def ClipRasters(self, coords):
        if coords is None or coords == '#':
            RasterName = self.paths.inputs.units
            rName = os.path.basename(RasterName)
            clippedName = self.paths.TMPDIR + rName
            self.gp.copy_management(RasterName, clippedName)
            self.paths.inputs.units = clippedName
            RasterName = self.paths.inputs.cellAreas
            rName = os.path.basename(RasterName)
            clippedName = self.paths.TMPDIR + rName
            self.gp.copy_management(RasterName, clippedName)
            self.paths.inputs.cellAreas = clippedName
        else:
            RasterName = self.paths.inputs.units
            rName = os.path.basename(RasterName)
            clippedName = self.paths.TMPDIR + rName
            self.gp.clip_management(RasterName, coords, clippedName)
            self.paths.inputs.units = clippedName
            RasterName = self.paths.inputs.cellAreas
            rName = os.path.basename(RasterName)
            clippedName = self.paths.TMPDIR + rName
            self.gp.clip_management(RasterName, coords, clippedName)
            self.paths.inputs.cellAreas = clippedName
        
        clippedLayerList = []
        
        for RasterName in self.paths.inputs.LayerList:
            rName = os.path.basename(RasterName)
            clippedName = self.paths.TMPDIR + rName
            if coords is None or coords == '#':
                self.gp.copy_management(RasterName, clippedName)
            else:
                self.gp.clip_management(RasterName, coords, clippedName)
            
            clippedLayerList.append(clippedName)
        
        self.paths.inputs.LayerList = clippedLayerList
                
    # Verify if rasters exist
    def VerifyRasters(self, ListOfRasters, gui):
        flag = True
        for RasterName in ListOfRasters:
            if type(RasterName) == list:
                self.VerifyRasters(RasterName, gui)
            else:
                if (not self.gp.Exists(RasterName)):
                    gui.Error('Raster does not exist: ' + RasterName)
                    raise Exception('Raster validation', 'Raster does not exitst')
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