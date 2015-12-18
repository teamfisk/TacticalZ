#ifndef WriteToFile_WriteToFile_h__
#define WriteToFile_WriteToFile_h__

#include "MayaIncludes.h"
#include "OutputData.h"
#include <fstream>
#include <string>

using namespace std;

class WriteToFile
{
public:
	~WriteToFile();
	bool binaryFilePath(string filePathAndFileName);
	bool ASCIIFilePath(string filePathAndFileName);

	void writeToFiles(OutputData* toWrite, unsigned int numOfElementToWrite = 1, unsigned int startIndex = 0)
	{
		MGlobal::displayInfo("WriteToFile::writeToFiles()");
		if (ASCIIFile.is_open())
		{
			MGlobal::displayInfo("Writing to ASCIIfile");
			for (unsigned int i = startIndex; i < numOfElementToWrite + startIndex; i++)
				ASCIIFile << toWrite[i] << endl;
		}

		if (binFile.is_open())
		{
			MGlobal::displayInfo("Writing to binaryfile");
			for (unsigned int i = startIndex; i < numOfElementToWrite + startIndex; i++)
				toWrite[i].WriteBinary(binFile);
		}

	}

	void OpenFiles();
	void CloseFiles();

private:
	string binFileName;
	string ASCIIFileName;
	ofstream binFile;
	ofstream ASCIIFile;
};



#endif