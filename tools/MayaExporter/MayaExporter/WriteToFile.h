#ifndef WriteToFile_WriteToFile_h__
#define WriteToFile_WriteToFile_h__

#include "MayaIncludes.h"
#include "OutputData.h"
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

class WriteToFile
{
public:
	~WriteToFile();
	bool binaryFilePath(string filePathAndFileName);
	bool ASCIIFilePath(string filePathAndFileName);

	void writeToFiles(OutputData* toWrite, unsigned int numOfElementToWrite = 1, unsigned int startIndex = 0)
	{
		MGlobal::displayInfo("WriteToFile::writeToFiles(OutputData*)");
		if (ASCIIFile.is_open())
		{
			for (unsigned int i = startIndex; i < numOfElementToWrite + startIndex; i++)
				ASCIIFile << toWrite[i] << endl;
		}

		if (binFile.is_open())
		{
			for (unsigned int i = startIndex; i < numOfElementToWrite + startIndex; i++)
				toWrite[i].WriteBinary(binFile);
		}

	}

    template <typename T>
    void writeToFiles(T* toWrite, unsigned int numOfElementToWrite = 1, unsigned int startIndex = 0)
    {
        MGlobal::displayInfo("WriteToFile::writeToFiles(T*) - Template T");
        if (ASCIIFile.is_open()) {
            for (unsigned int i = startIndex; i < numOfElementToWrite + startIndex; i++)
                ASCIIFile << toWrite[i] << endl;
        }

        if (binFile.is_open()) {
            for (unsigned int i = startIndex; i < numOfElementToWrite + startIndex; i++)
                binFile.write((char*)toWrite, sizeof(T));
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