@echo off
copy "d:\Workspace\IIASA\GeoProcessing2\gisTools\convertFloatToRaster.py" "D:\Workspace\IIASA\GeoProcessing2\Debug\convertFloatToRaster.py"
if errorlevel 1 goto VCReportError
goto VCEnd
:VCReportError
echo Project : error PRJ0019: A tool returned an error code from "Performing Post-Build Event..."
exit 1
:VCEnd