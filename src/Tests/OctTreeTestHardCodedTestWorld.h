#include <list>
#include <tuple>
#include <boost/any.hpp>
#include "GLM.h"
#include "Core/World.h"
#include "Core/Util/Any.h"

//octTree
//#include <windows.h>

//last!
#define private public
#include <Engine\Core\OctTree.h>

class HardcodedTestWorld : public World
{
public:
    EntityID OctTreeEntityIdSaved;

    //constructor
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
        //{
        //    EntityID entityScaleWidget = world.CreateEntity();
        //    ComponentWrapper transform = world.AttachComponent(entityScaleWidget, "Transform");
        //    transform["Position"] = glm::vec3(-1.5f, 0.f, 0.f);
        //    ComponentWrapper model = world.AttachComponent(entityScaleWidget, "Model");
        //    model["Resource"] = "Models/ScaleWidget.obj";
        //}
        //{
        //    EntityID entityRotationWidget = world.CreateEntity();
        //    ComponentWrapper transform = world.AttachComponent(entityRotationWidget, "Transform");
        //    transform["Position"] = glm::vec3(1.5f, 0.f, 0.f);
        //    ComponentWrapper model = world.AttachComponent(entityRotationWidget, "Model");
        //    model["Resource"] = "Models/RotationWidget.obj";
        //}
        //{
        //    EntityID entityDummyScene = world.CreateEntity();
        //    ComponentWrapper transform = world.AttachComponent(entityDummyScene, "Transform");
        //    transform["Position"] = glm::vec3(0, 0.f, 0.f);
        //    ComponentWrapper model = world.AttachComponent(entityDummyScene, "Model");
        //    model["Resource"] = "Models/DummyScene.obj";
        //}

        //add octTree
        {
            //ComponentWrapper debug = world.AttachComponent(e, "Debug");
            //// Set the Name field of the Debug component using subscript operator
            //debug["Name"] = "Blah";
            auto minCorner = glm::vec3(0.0f, 0.0f, 0.0f);
            auto maxCorner = glm::vec3(1.0f, 1.0f, 1.0f);
            auto someAABB = AABB(minCorner, maxCorner);

            auto someOctTree = OctTree(someAABB, 2);

            //auto min1 = someOctTree.m_Children[i]->m_Box.MinCorner();
            //auto max1 = someOctTree.m_Children[i]->m_Box.MaxCorner();

            float boxDrawFactor = 1.05f;
            auto halfSizeFactor = 0.0f;
            halfSizeFactor = someAABB.HalfSize().x;

            //the first box first
            EntityID entityDummyScene = world.CreateEntity();

            ComponentWrapper transform = world.AttachComponent(entityDummyScene, "Transform");
            transform["Position"] = someAABB.Center()*1.0f;
            transform["Scale"] = glm::vec3(1.0f, 1.0f, 1.0f)*halfSizeFactor*boxDrawFactor;

            ComponentWrapper model = world.AttachComponent(entityDummyScene, "Model");
            model["Resource"] = "Models/Core/UnitBox.obj";
            model["Color"] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

            //just draw the first 8 children - looks nicer in code if i split it this way
            for (size_t i = 0; i < 8; i++)
            {
                auto cen1 = someOctTree.m_Children[i]->m_Box.Center();
                halfSizeFactor = someOctTree.m_Children[i]->m_Box.HalfSize().x;

                EntityID entityDummyScene = world.CreateEntity();
                
                ComponentWrapper transform = world.AttachComponent(entityDummyScene, "Transform");
                transform["Position"] = cen1*1.0f;
                transform["Scale"] = glm::vec3(1.0f, 1.0f, 1.0f)*0.97f*halfSizeFactor*boxDrawFactor;

                ComponentWrapper model = world.AttachComponent(entityDummyScene, "Model");
                model["Resource"] = "Models/Core/UnitBox.obj";
                model["Color"] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

                //OutputDebugStringA((std::to_string(cen1.x) + std::to_string(cen1.y) + std::to_string(cen1.z)).c_str());
                //OctTreeEntityIdSaved = entityDummyScene;
            }
            
            //then draw the childrens children
            //for (size_t j = 0; j < 8; j++)
            //{
            //    auto someChild = someOctTree.m_Children[j];

            //    for (size_t i = 0; i < 8; i++)
            //    {
            //        auto cen1 = someChild->m_Children[i]->m_Box.Center();
            //        auto boxScale = someChild->m_Children[i]->m_Box.HalfSize();

            //        EntityID entityDummyScene = world.CreateEntity();

            //        ComponentWrapper transform = world.AttachComponent(entityDummyScene, "Transform");
            //        transform["Position"] = cen1*1.0f;
            //        transform["Scale"] = glm::vec3(1.0f, 1.0f, 1.0f)*0.97f*boxScale*boxDrawFactor;

            //        ComponentWrapper model = world.AttachComponent(entityDummyScene, "Model");
            //        model["Resource"] = "Models/Core/UnitBox.obj";
            //        model["Color"] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
            //        //OctTreeEntityIdSaved = entityDummyScene;
            //    }

            //}


        }


    }
};