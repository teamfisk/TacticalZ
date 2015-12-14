#ifndef SnapshotDefinitions_h__
#define SnapshotDefinitions_h__

struct SnapshotDefinitions
{
	// "+Forward" is 8 characters * sizeof(char) = 8
	std::string InputForward;
	// "+Right" is 6 characters * sizeof(char) = 6
    std::string InputRight;
};

struct IsWASDKeyDown
{
    bool W = false;
    bool A = false;
    bool S = false;
    bool D = false;
};

#endif