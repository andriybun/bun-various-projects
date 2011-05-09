# Date:   19/02/2009
# Author: Andriy Bun
# Name:   ...

#Import standard library modules
import arcgisscripting, os #, sys, string, time
from utils import utils

gp = arcgisscripting.create()
gp.CheckOutExtension("Spatial")
gp.OverWriteOutput = 1

PrintMessageTime("Started")

#-------------------------------------------------------------------------------
# Default parameters of the run:
#-------------------------------------------------------------------------------
# Current directory where the script is located
HOMEDIR = os.getcwd() + "\\"

# Input folder. Move all the rasters and all data to this directory:
INPUTDIR = "input\\"

mp = HOMEDIR + INPUTDIR + "countries"
try:
    gp.delete_management(mp + "_cp")
except:
    print ""
gp.copy_management(mp, mp + "_cp")

gp.BuildRasterAttributeTable_management(mp + "_cp","OVERWRITE")

gp.Workspace = mp + "_cp"

gp.addfield (mp + "_cp","MYFIELD","LONG", "#", "#", "#", "#", "NULLABLE", "REQUIRED", "#")

rows = gp.UpdateCursor(mp + "_cp","","","","VALUE")
row = rows.next()
while row:
    row.setValue("MYFIELD", row.VALUE * 10)
    rows.UpdateRow(row)
    row = rows.Next()

gp.ExtractByAttributes_sa(mp + "_cp.MYFIELD", "MYFIELD > 0", INPUTDIR + "temp1")

del row
del rows
