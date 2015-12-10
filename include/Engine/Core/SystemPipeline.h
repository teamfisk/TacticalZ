#ifndef SystemPipeline_h__
#define SystemPipeline_h__

#include "../Common.h"
#include "EventBroker.h"
#include "System.h"
#include "World.h"

class SystemPipeline
{
public:
    SystemPipeline(const EventBroker* eventBroker)
        : m_EventBroker(eventBroker)
    { }
    ~SystemPipeline()
    {
        for (auto& pair : m_Systems) {
            for (auto& system : pair.second) {
                delete system;
            }
        }
    }

    template <typename T, typename... Arguments>
    void AddSystem(Arguments... args)
    {
        System* system = new T(m_EventBroker, args...);
        if (!system->m_ComponentType.empty()) {
            m_Systems[system->m_ComponentType].push_back(system);
        } else {
            LOG_ERROR("Failed to add system \"%s\": Missing component type!", typeid(T).name());
            delete system;
        }
    }

    void Update(World* world, double dt)
    {
        for (auto& pair : m_Systems) {
            const std::string& componentName = pair.first;
            auto& systems = pair.second;
            const ComponentPool* pool = world->GetComponents(componentName);
            if (pool == nullptr) {
                continue;
            }
            for (auto& component : *pool) {
                for (auto& system : systems) {
                    system->Update(world, component, dt);
                }
            }
        }
    }

private:
    const EventBroker* m_EventBroker;
    std::unordered_map<std::string, std::vector<System*>> m_Systems;
};

#endif