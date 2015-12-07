#include <list>
#include <tuple>
#include <boost/any.hpp>
#include "GLM.h"
#include "Core/World.h"

struct any
{
    any() { }

    template <typename T>
    any(const T& value)
    {
        Buffer = std::shared_ptr<char>(new char[sizeof(T)]);
        memcpy(Buffer.get(), &value, sizeof(T));
    }

    template <typename T>
    any(T&& value)
    {
        Buffer = std::shared_ptr<char>(new char[sizeof(T)]);
        memcpy(Buffer.get(), &value, sizeof(T));
    }

    template <typename T>
    any& operator=(const T& value)
    {
        return any(value);
    }

    template <typename T>
    any& operator=(T&& value)
    {
        return any(value);
    }

    std::shared_ptr<char> Buffer = nullptr;
};

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
        std::unordered_map<std::string, std::list<std::tuple<std::size_t, std::string, any>>> components
        {
            {
                "Test", 
                { 
                    std::make_tuple(sizeof(int), "TestInteger", 1337),
                    std::make_tuple(sizeof(float), "TestFloat", 13.37f) 
                } 
            },
            {
                "Debug",
                {
                    std::make_tuple(sizeof(std::string), "Name", std::string("Unnamed")),
                    std::make_tuple(sizeof(glm::vec3), "PickingColor", glm::vec3(0.f))
                }
            },
            {
                "Transform", 
                { 
                    std::make_tuple(sizeof(glm::vec3), "Position", glm::vec3(0.f, 0.f, 0.f)),
                    std::make_tuple(sizeof(glm::quat), "Orientation", glm::quat()),
                    std::make_tuple(sizeof(glm::vec3), "Scale", glm::vec3(1.f, 1.f, 1.f))
                } 
            },
            {
                "Model",
                {
                    std::make_tuple(sizeof(std::string), "ModelFile", std::string("Unnamed")),
                    std::make_tuple(sizeof(glm::vec4), "Color", glm::vec4(1.f, 1.f, 1.f, 1.f)),
                }
            }
        };

        for (auto& c : components) {
            ComponentInfo ci;
            ci.Name = c.first;

            // Fields
            unsigned int stride = 0;
            for (auto& f : c.second) {
                stride += std::get<0>(f);
            }
            ci.Meta.Stride = stride;
            ci.Defaults = std::shared_ptr<char>(new char[stride]);
            unsigned int offset = 0;
            for (auto& f : c.second) {
                std::size_t size;
                std::string fieldName;
                any defaultValue;
                std::tie(size, fieldName, defaultValue) = f;

                ci.FieldOffsets[fieldName] = offset;
                ci.FieldTypes[fieldName] = "undefined";
                memcpy(ci.Defaults.get() + offset, defaultValue.Buffer.get(), size);
                offset += size;
            }

            RegisterComponent(ci);
        }
    }

    void createTestEntities()
    {
        {
            EntityID e = CreateEntity();
            AttachComponent(e, "Test");
            AttachComponent(e, "Debug");
            AttachComponent(e, "Transform");
            ComponentWrapper testComponent = GetComponent(e, "Test");
            int testValue = testComponent["TestInteger"];
            float testFloat = testComponent["TestFloat"];
            ComponentWrapper debugComponent = GetComponent(e, "Debug");
            std::string name = debugComponent["Name"];
            glm::vec3 pickingColor = debugComponent["PickingColor"];
            ComponentWrapper testTransform = GetComponent(e, "Transform");
            glm::vec3 pos = testTransform["Position"];
            glm::quat ori = testTransform["Orientation"];
            glm::vec3 scale = testTransform["Scale"];
        }

        {
            EntityID entityTranslationWidget = CreateEntity();
            AttachComponent(entityTranslationWidget, "Transform");
            AttachComponent(entityTranslationWidget, "Model");
            ComponentWrapper testTransform = GetComponent(entityTranslationWidget, "Transform");
            glm::vec3 pos = testTransform["Position"];
            glm::quat ori = testTransform["Orientation"];
            glm::vec3 scale = testTransform["Scale"];
            ComponentWrapper testModel = GetComponent(entityTranslationWidget, "Model");
            std::string file = testModel["ModelFile"];
            testModel["ModelFile"] = "Models/TranslationWidget.obj";
            glm::vec4 col = testModel["Color"];
        }
        {
            EntityID entityScaleWidget = CreateEntity();
            AttachComponent(entityScaleWidget, "Transform");
            AttachComponent(entityScaleWidget, "Model");
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