#include <list>
#include <tuple>
#include <boost/any.hpp>
#include "GLM.h"
#include "Core/World.h"
#include "Core/Util/Any.h"
#include "Core/EntityXMLFile.h"

class HardcodedTestWorld : public World
{
public:
    HardcodedTestWorld()
        : World()
    {
        ResourceManager::RegisterType<EntityXMLFile>("EntityXMLFile");
        ResourceManager::Load<EntityXMLFile>("Schema/Entities/Test.xml")->PopulateWorld(this);
        
        createTestEntities();
    }

private:
    void createTestEntities()
    {
        //Create some test widgets
        {
            EntityID entityScaleWidget = CreateEntity();
            ComponentWrapper transform = AttachComponent(entityScaleWidget, "Transform");
            transform["Position"] = glm::vec3(-1.5f, 0.f, 0.f);
            ComponentWrapper model = AttachComponent(entityScaleWidget, "Model");
            model["Resource"] = "Models/ScaleWidget.obj";
        }
        {
            EntityID entityRotationWidget = CreateEntity();
            ComponentWrapper transform = AttachComponent(entityRotationWidget, "Transform");
            transform["Position"] = glm::vec3(1.5f, 0.f, 0.f);
            ComponentWrapper model = AttachComponent(entityRotationWidget, "Model");
            model["Resource"] = "Models/RotationWidget.obj";
        }
        {
            EntityID entityDummyScene = CreateEntity();
            ComponentWrapper transform = AttachComponent(entityDummyScene, "Transform");
            transform["Position"] = glm::vec3(0, 0.f, 0.f);
            ComponentWrapper model = AttachComponent(entityDummyScene, "Model");
            model["Resource"] = "Models/DummyScene.obj";
        }
    }
};