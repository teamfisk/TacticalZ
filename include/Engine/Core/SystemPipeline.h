#ifndef SystemPipeline_h__
#define SystemPipeline_h__

#include "../Common.h"
#include "EventBroker.h"
#include "System.h"
#include "World.h"

class SystemPipeline
{
public:
    SystemPipeline(EventBroker* eventBroker)
        : m_EventBroker(eventBroker)
    { }
    ~SystemPipeline()
    {
        for (auto& pair : m_PureSystems) {
            for (auto& system : pair.second) {
                delete system;
            }
        }
    }

    template <typename T, typename... Arguments>
    void AddSystem(Arguments... args)
    {
        System* system = new T(m_EventBroker, args...);
        m_Systems[typeid(T).name()] = system;

        if (std::is_base_of<PureSystem, T>::value) {
            PureSystem* pureSystem = static_cast<PureSystem*>(system);
            if (!pureSystem->m_ComponentType.empty()) {
                m_PureSystems[pureSystem->m_ComponentType].push_back(pureSystem);
            } else {
                LOG_ERROR("Failed to add pure system \"%s\": Missing component type!", typeid(T).name());
            }
        }

        if (std::is_base_of<ImpureSystem, T>::value) {
            ImpureSystem* impureSystem = static_cast<ImpureSystem*>(system);
            m_ImpureSystems.push_back(impureSystem);
        }
    }

    void Update(World* world, double dt)
    {
        // Process events
        for (auto& pair : m_Systems) {
            m_EventBroker->Process(pair.first);
        }

        // Update
        for (auto& pair : m_PureSystems) {
            const std::string& componentName = pair.first;
            auto& systems = pair.second;
            const ComponentPool* pool = world->GetComponents(componentName);
            if (pool == nullptr) {
                continue;
            }
            for (auto& component : *pool) {
                for (auto& system : systems) {
                    system->UpdateComponent(world, component, dt);
                }
            }
        }
        for (auto& system : m_ImpureSystems) {
            system->Update(world, dt);
        }
    }

private:
    EventBroker* m_EventBroker;
    std::map<std::string, System*> m_Systems;
    std::map<std::string, std::vector<PureSystem*>> m_PureSystems;
    std::vector<ImpureSystem*> m_ImpureSystems;
};

#endif