import sys
import os
import shutil
import hashlib

def generateProjectFiles(templateFileName, generatedFileName, oldStr, newStr):
    templateFile = open(templateFileName, 'r')
    tfLines = templateFile.readlines()
    templateFile.close()
    generatedFile = open(generatedFileName, 'w')
    for line in tfLines:
        generatedFile.write(line.replace(oldStr, newStr))
    generatedFile.close()
    
def addProjectToSolution(NEW_PROJECT_NAME, guid):
    solutionFileOrig = open("GeoProcessing2.sln", 'r')
    solText = solutionFileOrig.readlines()
    solutionFileOrig.close()
    
    shutil.move("GeoProcessing2.sln", "GeoProcessing2.sln.bak")
    solutionFile = open("GeoProcessing2.sln", 'w')
    
    textToAdd = [\
        "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%s\", \"%s\\%s.vcxproj\", \"{%s}\"\n" \
        % (NEW_PROJECT_NAME, NEW_PROJECT_NAME, NEW_PROJECT_NAME, guid), \
        "\tProjectSection(ProjectDependencies) = postProject\n", \
        "\t\t{E5DA2D38-E736-4619-8D0B-D4DE27F34BD7} = {E5DA2D38-E736-4619-8D0B-D4DE27F34BD7}\n", \
        "\t\t{DE3FABA0-FF0D-46D2-92F0-BC5F207FD5D8} = {DE3FABA0-FF0D-46D2-92F0-BC5F207FD5D8}\n", \
        "\tEndProjectSection\n", \
        "EndProject\n"]
    
    idx = 1
    line = solText[idx]
    
    while (line != "Global\n"):
        solutionFile.write(line)
        line = solText[idx]
        idx += 1

    solutionFile.writelines(textToAdd)
    
    while (line != "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n"):
        solutionFile.write(line)
        line = solText[idx]
        idx += 1
        
    while (line != "\tEndGlobalSection\n"):
        solutionFile.write(line)
        line = solText[idx]
        idx += 1

    solutionFile.write("\t\t{%s}.Debug|Win32.ActiveCfg = Debug|Win32\n" % (guid))
    solutionFile.write("\t\t{%s}.Debug|Win32.Build.0 = Debug|Win32\n" % (guid))
    solutionFile.write("\t\t{%s}.Release|Win32.ActiveCfg = Release|Win32\n" % (guid))
    solutionFile.write("\t\t{%s}.Release|Win32.Build.0 = Release|Win32\n" % (guid))
    
    while (idx < len(solText)):
        solutionFile.write(line)
        line = solText[idx]
        idx += 1
    solutionFile.write(line)
    solutionFile.close()

    
if __name__ == "__main__":
    NEW_PROJECT_NAME = sys.argv[1]
    
    guid = hashlib.md5(NEW_PROJECT_NAME.encode('utf-8')).hexdigest()
    guid = "%s-%s-%s-%s-%s" % (guid[0:8], guid[8:12], guid[12:16], guid[16:20], guid[20:])
    guid = guid.upper()
	
    # Create directory for new project
    if not os.path.exists(NEW_PROJECT_NAME):
        os.mkdir(NEW_PROJECT_NAME)
    else:
        raise Exception("Specified directory already exists.")

    # List of files to process
    fileListRaw = [ \
        "mainNEW_PROJECT_TEMPLATE.cpp" , \
        "mainNEW_PROJECT_TEMPLATE.py", \
        "NEW_PROJECT_TEMPLATE.vcxproj"]
    fileList = []

    # Generate 
    for fileName in fileListRaw:
        # Path to template file
        templateFileName = "NEW_PROJECT_TEMPLATE\\%s" % (fileName)
        
        # Some magic with file names
        new_project_name_cap = NEW_PROJECT_NAME[0].upper() + NEW_PROJECT_NAME[1:]
        generatedFileName = fileName.replace("NEW_PROJECT_TEMPLATE", new_project_name_cap)
        generatedFileName = generatedFileName[0].lower() + generatedFileName[1:]
        # Full path to generated file
        generatedFileName = "%s\\%s" % (NEW_PROJECT_NAME, generatedFileName)
        
        # Generate files
        generateProjectFiles(templateFileName, generatedFileName, "NEW_PROJECT_TEMPLATE", NEW_PROJECT_NAME);
        generateProjectFiles(generatedFileName, generatedFileName, "PROJECT_GUID", guid);

        fileList.append(generatedFileName)
    
    # Add project to solution
    addProjectToSolution(NEW_PROJECT_NAME, guid)

    """
    Now parsing names of parameters passed as command line arguments from 2nd to the end
    """
    nArgs = len(sys.argv) - 1
    EXECUTE_COMMAND_FORMAT = '\'' + r'"%s" --workingDir "%s" --resultDir "%s" --tmpDir "%s"'
    EXECUTE_COMMAND_PARAMS = ""
    OPTIONS_DESCRIPTION = ""
    PARSE_PARAMS_DECLARATION = ""
    PARSE_PARAMS = ""
    i = 0
    for parName in sys.argv[2:]:
        i += 1
        EXECUTE_COMMAND_FORMAT += r' "%s" --' + parName
        EXECUTE_COMMAND_PARAMS += ', \\\n\t\tsys.argv[%d]' % (i)
        OPTIONS_DESCRIPTION += "\t\t(\"%s\", po::value<#TYPE_OF_PARAMETER_%d>(), \"ADD DESCRIPTION\")\n" % (parName, i)
        PARSE_PARAMS_DECLARATION += "\t#TYPE_OF_PARAMETER_%d #NAME_OF_PARAMETER_%d;\n" % (i, i)
        PARSE_PARAMS += \
            	"\tif (vm.count(\"%s\"))\n" % (parName) + \
	            "\t{\n" + \
		        "\t\t#NAME_OF_PARAMETER_%d = vm[\"%s\"].as<#TYPE_OF_PARAMETER_%d>();\n" % (i, parName, i) + \
		        "\t\tnParams++;\n" + \
	            "\t}\n"


    EXECUTE_COMMAND_FORMAT += '\''
    generateProjectFiles(fileList[1], fileList[1], "EXECUTE_COMMAND_FORMAT", EXECUTE_COMMAND_FORMAT)
    generateProjectFiles(fileList[1], fileList[1], "EXECUTE_COMMAND_PARAMS", EXECUTE_COMMAND_PARAMS)
    generateProjectFiles(fileList[0], fileList[0], "#OPTIONS_DESCRIPTION", OPTIONS_DESCRIPTION)
    generateProjectFiles(fileList[0], fileList[0], "#PARSE_PARAMS_DECLARATION", PARSE_PARAMS_DECLARATION)
    generateProjectFiles(fileList[0], fileList[0], "#PARSE_PARAMS", PARSE_PARAMS)
    generateProjectFiles(fileList[0], fileList[0], "#NUM_PARAMS", str(i))

    print("Project files generated successfully")