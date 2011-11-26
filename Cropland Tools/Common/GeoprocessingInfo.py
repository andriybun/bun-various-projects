# -*- coding: utf-8 -*-
"""
Created on Mon Nov 29 14:03:29 2010

@author: Andriy Bun

@name:   
"""

import time
import arcgisscripting
import warnings

# Class interface for updating status in ArcGIS
class GeoprocessingInfo():
    def __init__(self):
        self.gp = arcgisscripting.create()
    
    # print status with time
    def PrintTextTime(self, textToAppend):
        self.gp.AddMessage(time.strftime("%H:%M:%S", time.localtime()) + '\t' + textToAppend)
        
    # print status
    def PrintText(self, textToAppend):
        self.gp.AddMessage(textToAppend)
    
    # set default progress bar
    def InitialiseDefaultProgressor(self, label):
        self.gp.SetProgressor("default", label)
        
    # set step progress bar
    def InitialiseStepProgressor(self, label):
        self.gp.SetProgressor("step", label, 1, 100, 1)

    # update progress bar position
    def SetProgress(self, value):
        self.gp.SetProgressorPosition(value)
        
    def Warning(self, msg):
        self.gp.AddWarning(msg)
    
    def Error(self, msg):
        self.gp.AddError(msg)
        raise Exception(msg)


#########################################################
##  No ArcGIS debug version
#########################################################
class GeoprocessingInfo_debug():
    def __init__(self):
        self.gp = arcgisscripting.create()
    
    # print status with time
    def PrintTextTime(self, textToAppend):
        print time.strftime("%H:%M:%S", time.localtime()) + '\t' + textToAppend
        
    # print status
    def PrintText(self, textToAppend):
        print textToAppend
    
    # set default progress bar
    def InitialiseDefaultProgressor(self, label):
        print label
        
    # set step progress bar
    def InitialiseStepProgressor(self, label):
        print label

    # update progress bar position
    def SetProgress(self, value):
        print str(value) + '%'
        
    def Warning(self, msg):
        warnings.warn(msg)

    def Error(self, msg):
        raise Exception(msg)

