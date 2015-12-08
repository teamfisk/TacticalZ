#include "ResourceManager.h"

class EntityFile : public Resource
{
	friend class ResourceManager;

private:
	EntityFile(std::string path);

public:

};