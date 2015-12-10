#ifndef SnapshotDefinitions_h__
#define SnapshotDefinitions_h__

struct SnapshotDefinitions
{
	// "+Forward" is 8 characters * sizeof(char) = 8
	char* InputForward = new char[8];
	// "+Right" is 6 characters * sizeof(char) = 6
	char* InputRight = new char[6];
};

struct IsWASDKeyDown
{
    bool W = false;
    bool A = false;
    bool S = false;
    bool D = false;
};

#endif