#ifndef Entity_h__
#define Entity_h__

typedef unsigned int EntityID;
const static unsigned int EntityID_Invalid = -1;

class World;
struct EntityWrapper
{
    EntityWrapper(::World* world, EntityID id)
        : World(world)
        , ID(id)
    { }

    ::World* World;
    EntityID ID;

    bool operator==(const EntityWrapper& e)
    {
        return (this->World == e.World) && (this->ID == e.ID);
    }
    operator EntityID() { return this->ID; }
};

#endif