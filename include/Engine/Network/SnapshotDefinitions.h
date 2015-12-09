#ifndef SnapshotDefinitions_h__
#define SnapshotDefinitions_h__

struct SnapshotDefinitions
{
	// "+Forward" is 8 characters * sizeof(char) = 8
	char* inputForward = new char[8];
	// "+Right" is 6 characters * sizeof(char) = 6
	char* inputRight = new char[6];
};

#endif