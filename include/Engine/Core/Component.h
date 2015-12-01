#ifndef Component_h__
#define Component_h__

#include "../Common.h"
#include "ComponentInfo.h"

struct Component
{
    Component(const unsigned int entityID, const ComponentInfo* componentInfo, char* data)
        : EntityID(entityID)
        , Info(componentInfo)
        , Data(data)
    { }

    const unsigned int EntityID;
    const ComponentInfo* Info;
    char* Data;

    template <typename T>
    T& Property(std::string name)
    {
        unsigned int offset=Info->FieldOffsets.at(name);
        return *reinterpret_cast<T*>(&Data[offset]);
    }

    template <typename T>
    void SetProperty(std::string name, T& value) { Property<T>(name)=value; }
    template <typename T>
    void SetProperty(std::string name, const T value) { Property<T>(name)=value; }

    // Specialization for string literals
    template <std::size_t N>
    void SetProperty(std::string name, const char(&value)[N]) { Property<std::string>(name)=std::string(value); }

    struct SubscriptProxy
    {
        friend struct Component;
    private:
        SubscriptProxy(Component* component, std::string& propertyName)
            : m_Component(component)
            , m_PropertyName(propertyName)
        { }

        Component* m_Component;
        std::string& m_PropertyName;

    public:
        template <typename T>
        operator T&() { return m_Component->Property<T>(m_PropertyName); }

        template <typename T>
        void operator=(const T val) { m_Component->SetProperty<T>(m_PropertyName, val); }
        template <typename T>
        void operator=(T& val) { m_Component->SetProperty<T>(m_PropertyName, val); }

        // Specialization for string literals
        template<std::size_t N>
        void operator=(const char(&val)[N]) { m_Component->SetProperty<N>(m_PropertyName, val); }
    };
    SubscriptProxy operator[](std::string propertyName) { return SubscriptProxy(this, propertyName); }
};

#endif