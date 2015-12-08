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
        registerTestComponents();
        createTestEntities();
    }

private:
    void registerTestComponents()
    {
        ComponentWrapperFactory f;

        f = ComponentWrapperFactory("Test");
        f.AddProperty("TestInteger", 1337);
        f.AddProperty("TestFloat", 13.37f);
        f.AddProperty("TestString", std::string("Carlito"));
        RegisterComponent(f);

        f = ComponentWrapperFactory("Debug");
        f.AddProperty("Name", std::string("Unnamed"));
        RegisterComponent(f);

        //f = ComponentWrapperFactory("Transform");
        //f.AddProperty("Position", glm::vec3(0.f, 0.f, 0.f));
        //f.AddProperty("Orientation", glm::quat());
        //f.AddProperty("Scale", glm::vec3(1.f, 1.f, 1.f));
        //RegisterComponent(f);

        f = ComponentWrapperFactory("Model");
        f.AddProperty("Resource", std::string());
        f.AddProperty("Color", glm::vec4(1.f, 1.f, 1.f, 1.f));
        f.AddProperty("Visible", true);
        RegisterComponent(f);
    }

    void createTestEntities()
    {
        ResourceManager::RegisterType<EntityXMLFile>("EntityXMLFile");
        ResourceManager::Load<EntityXMLFile>("Schema/Entities/Test.xml")->PopulateWorld(this);

        World& world = *this;

        // Create an entity
        EntityID e = world.CreateEntity();

        // Attach a Transform component
        world.AttachComponent(e, "Transform");
        // Fetch the component based on EntityID and component type
        ComponentWrapper transform = world.GetComponent(e, "Transform");
        // Set the fields of the Transform component
        transform["Position"] = glm::vec3(0.f, 0.f, 0.f);
        transform["Scale"] = glm::vec3(1.f, 1.f, 1.f);

        // Move on the X axis by fetching field as reference
        ((glm::vec3&)transform["Position"]).x += 10.f;
        // Shrink by a factor of 100
        ((glm::vec3&)transform["Scale"]) /= 100.f;

        // Loop through all Transform components and print them
        for (auto& transform : world.GetComponents("Transform")) {
            glm::vec3 pos = transform["Position"];
            std::cout << "Position: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
            glm::vec3 scale = transform["Scale"];
            std::cout << "Scale: " << scale.x << " " << scale.y << " " << scale.z << std::endl;
        }
    }
};