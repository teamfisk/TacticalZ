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

        f = ComponentWrapperFactory("Collision");
        f.AddProperty("BoxCenter", glm::vec3(0.f, 0.f, 0.f));
        f.AddProperty("BoxSize", glm::vec3(1.f, 1.f, 1.f));
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
        
        //Create some test widgets
        {
            EntityID entityScaleWidget = world.CreateEntity();
            ComponentWrapper transform = world.AttachComponent(entityScaleWidget, "Transform");
            transform["Position"] = glm::vec3(-1.5f, 0.f, 0.f);
            ComponentWrapper model = world.AttachComponent(entityScaleWidget, "Model");
            model["Resource"] = "Models/ScaleWidget.obj";
        }
        {
            EntityID entityRotationWidget = world.CreateEntity();
            ComponentWrapper transform = world.AttachComponent(entityRotationWidget, "Transform");
            transform["Position"] = glm::vec3(1.5f, 0.f, 0.f);
            ComponentWrapper model = world.AttachComponent(entityRotationWidget, "Model");
            model["Resource"] = "Models/RotationWidget.obj";
        }
        {
            EntityID entityDummyScene = world.CreateEntity();
            ComponentWrapper transform = world.AttachComponent(entityDummyScene, "Transform");
            transform["Position"] = glm::vec3(0, 0.f, 0.f);
            ComponentWrapper model = world.AttachComponent(entityDummyScene, "Model");
            model["Resource"] = "Models/DummyScene.obj";
        }
        {
            EntityID entityCollisionBox = world.CreateEntity();
            ComponentWrapper transform = world.AttachComponent(entityCollisionBox, "Transform");
            transform["Position"] = glm::vec3(0.f, 2.f, 0.f);
            ComponentWrapper model = world.AttachComponent(entityCollisionBox, "Model");
            model["Resource"] = "Models/Core/UnitBox.obj";

            ComponentWrapper collision = world.AttachComponent(entityCollisionBox, "Collision");
            glm::vec3 pos = transform["Position"];
            glm::vec3 scale = transform["Scale"];
            glm::quat ori = transform["Orientation"];
            Model* modelRes = ResourceManager::Load<Model>(model["Resource"]);

            //WTODO: This only works for objects that never moves/scales/rotates since modelMatrix don't change.
            //For dynamic objects, should save a collision box in modelspace and transform box with modelMatrix per collision check.
            glm::mat4 modelMatrix = modelRes->m_Matrix * glm::translate(glm::mat4(), pos) * glm::toMat4(ori) * glm::scale(scale);

            glm::vec3 mini = glm::vec3(INFINITY, INFINITY, INFINITY);
            glm::vec3 maxi = glm::vec3(-INFINITY, -INFINITY, -INFINITY);
            for (const auto& v : modelRes->m_Vertices) {
                const auto& wPos =  modelMatrix * glm::vec4(v.Position.x, v.Position.y, v.Position.z, 1);
                maxi.x = std::max(wPos.x, maxi.x);
                maxi.y = std::max(wPos.y, maxi.y);
                maxi.z = std::max(wPos.z, maxi.z);
                mini.x = std::min(wPos.x, mini.x);
                mini.y = std::min(wPos.y, mini.y);
                mini.z = std::min(wPos.z, mini.z);
            }
            collision["BoxCenter"] = 0.5f * (maxi + mini);
            collision["BoxSize"] = maxi - mini;
        }



    }
};