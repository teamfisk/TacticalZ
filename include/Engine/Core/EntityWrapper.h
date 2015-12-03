#ifndef Entity_h__
#define Entity_h__

typedef unsigned int EntityID;

struct EntityWrapper
{
    EntityWrapper(EntityID entityID)
        : ID(entityID)
    { }

	EntityID ID;
};

#endif