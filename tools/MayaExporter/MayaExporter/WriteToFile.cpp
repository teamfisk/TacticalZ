#include "WriteToFile.h"

WriteToFile::~WriteToFile()
{
	CloseFiles();
}

bool WriteToFile::binaryFilePath(string filePathAndFileName)
{
	binFileName = filePathAndFileName;
	ofstream binFile(filePathAndFileName, ofstream::binary);
	if (!binFile)
		return false;
	return true;
}

bool WriteToFile::ASCIIFilePath(string filePathAndFileName)
{
	ASCIIFileName = filePathAndFileName;
	ofstream ASCIIFile(filePathAndFileName);
	if (!ASCIIFile)
		return false;
	return true;
}

void WriteToFile::OpenFiles()
{
	if (binFile)
		binFile.open(binFileName, ofstream::binary);
	if (ASCIIFile)
		ASCIIFile.open(ASCIIFileName);
}

void WriteToFile::CloseFiles()
{
	binFile.close();
	ASCIIFile.close();
}