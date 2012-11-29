#include "testXmlParser.h"

int main(int argc, char * argv[])
{
	if (argc < 2)
	{
		printf("Error! File name not specified.\n");
		return 1;
	}

	string fileName = argv[1];

	xmlParser pathsParser(fileName);
	pathsParser.printXmlTree();
	vector<string> result;
	vector<xmlData::paramsMapT> resultParams;
	pathsParser.getSubtags("struct", result, resultParams, pair<string, string>("name", "inCellDataT"), "field");

	//string value;
	//xmlParser::executionResultT execResult;
	//map<string, string> paramsMap;
	//execResult = pathsParser.getValue("output_dir", value, paramsMap);
	//if (execResult == xmlParser::EXECUTION_RESULT_OK) cout << value << endl;
	//execResult = pathsParser.getValue("input_dir", value, paramsMap);
	//if (execResult == xmlParser::EXECUTION_RESULT_OK) cout << value << endl;
	//execResult = pathsParser.getValue("simu_bin_file", value, paramsMap);
	//if (execResult == xmlParser::EXECUTION_RESULT_OK) cout << value << endl;

	//xmlData tree;
	//tree.branchPush("branch1");
	//tree.navigateUp();
	//tree.branchPush("branch2");
	////tree.navigateUp();
	////tree.addBranch("branch2");
	//tree.navigatePrev();
	//tree.navigateNext();
	//tree.navigateUp();
	//tree.navigateDown();
	//tree.navigateUp();
	//tree.navigateDown(2);

	return 0;
}

