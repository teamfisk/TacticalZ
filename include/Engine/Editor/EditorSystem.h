#include "../Core/System.h"

class EditorSystem : public ImpureSystem
{
public:
    EditorSystem(EventBroker* eventBroker)
        : ImpureSystem(eventBroker)
    { }

    virtual void Update(World* world, double dt) override;
};