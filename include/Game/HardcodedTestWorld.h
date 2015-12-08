#include <list>
#include <tuple>
#include <boost/any.hpp>
#include "GLM.h"
#include "Core/World.h"
#include "Core/Util/Any.h"

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
            },
            {
                "Model",
                {
                    std::make_tuple(sizeof(std::string), "ModelFile", std::string("Unnamed")),
                    std::make_tuple(sizeof(glm::vec4), "Color", glm::vec4(1.f, 1.f, 1.f, 1.f)),
                }

        f = ComponentWrapperFactory("Test");
        f.AddProperty("TestInteger", 1337);
        f.AddProperty("TestFloat", 13.37f);
        f.AddProperty("TestString", std::string("Carlito"));
        RegisterComponent(f);

        f = ComponentWrapperFactory("Debug");
        f.AddProperty("Name", std::string("Unnamed"));
        RegisterComponent(f);

        f = ComponentWrapperFactory("Transform");
        f.AddProperty("Position", glm::vec3(0.f, 0.f, 0.f));
        f.AddProperty("Orientation", glm::quat());
        f.AddProperty("Scale", glm::vec3(1.f, 1.f, 1.f));
        RegisterComponent(f);

        f = ComponentWrapperFactory("Model");
        f.AddProperty("Resource", std::string());
        f.AddProperty("Color", glm::vec4(1.f, 1.f, 1.f, 1.f));
        f.AddProperty("Visible", true);
        RegisterComponent(f);
    }

    void createTestEntities()
    {
        World& world = *this;

        // Create an entity
        EntityID e = world.CreateEntity();

        // Attach a Debug component
        ComponentWrapper debug = world.AttachComponent(e, "Debug");
        // Set the Name field of the Debug component using subscript operator
        debug["Name"] = "Carlito";

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

            // Fetch the Debug component also present in this entity
            ComponentWrapper debug = world.GetComponent(transform.EntityID, "Debug");
            std::cout << "Name: " << (std::string)debug["Name"] << std::endl;
        }
            ComponentWrapper testTransform = GetComponent(entityScaleWidget, "Transform");
            testTransform["Position"] = glm::vec3(-1.5f, 0.f, 0.f);
            glm::quat ori = testTransform["Orientation"];
            glm::vec3 scale = testTransform["Scale"];
            ComponentWrapper testModel = GetComponent(entityScaleWidget, "Model");
            testModel["ModelFile"] = "Models/ScaleWidget.obj";
            glm::vec4 col = testModel["Color"];
        }
        {
            EntityID entityRotationWidget = CreateEntity();
            AttachComponent(entityRotationWidget, "Transform");
            AttachComponent(entityRotationWidget, "Model");
            ComponentWrapper testTransform = GetComponent(entityRotationWidget, "Transform");
            testTransform["Position"] = glm::vec3(1.5f, 0.f, 0.f);
            glm::quat ori = testTransform["Orientation"];
            glm::vec3 scale = testTransform["Scale"];
            ComponentWrapper testModel = GetComponent(entityRotationWidget, "Model");
            testModel["ModelFile"] = "Models/RotationWidget.obj";
        }



    }
};