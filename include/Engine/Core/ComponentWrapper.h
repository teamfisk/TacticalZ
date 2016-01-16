#ifndef ComponentWrapper_h__
#define ComponentWrapper_h__

#include "../Common.h"
#include "Entity.h"
#include "ComponentInfo.h"
#include "Util/Any.h"

// TODO: Change all instances "Property" to "Field" to remain consistent with ComponentInfo
struct ComponentWrapper
{
    ComponentWrapper(const ComponentInfo& componentInfo, char* data)
        : Info(componentInfo)
        , EntityID(*reinterpret_cast<::EntityID*>(data))
        , Data(data + sizeof(::EntityID))
    { }

    const ComponentInfo& Info;
    const ::EntityID EntityID;
    char* Data;

    int Enum(const char* fieldName, const char* enumKey)
    {
        return Info.Meta->FieldEnumDefinitions.at(fieldName).at(enumKey);
    }

    template <typename T>
    T& Property(std::string name)
    {
        unsigned int offset = Info.Fields.at(name).Offset;
        return *reinterpret_cast<T*>(&Data[offset]);
    }

    template <typename T>
    void SetProperty(std::string name, const T value) { Property<T>(name) = value; }
    //template <typename T>
    //void SetProperty(std::string name, T& value) { Property<T>(name) = value; }

    // Specialization for string literals
    template <std::size_t N>
    void SetProperty(std::string name, const char(&value)[N]) { Property<std::string>(name) = std::string(value); }
    
    struct SubscriptProxy
    {
        friend struct ComponentWrapper;
    private:
        SubscriptProxy(ComponentWrapper* component, std::string propertyName)
            : m_Component(component)
            , m_PropertyName(propertyName)
        { }

        ComponentWrapper* m_Component;
        std::string m_PropertyName;

    public:
        // Return the integer value of an enum type key for this field
        int Enum(const char* enumKey) { return m_Component->Enum(m_PropertyName.c_str(), enumKey); }

        template <typename T>
        operator T&() { return m_Component->Property<T>(m_PropertyName); }

        template <typename T>
        void operator=(const T val) { m_Component->SetProperty<T>(m_PropertyName, val); }
        // TODO: Pass by reference and rvalue (universal reference?)
        //template <typename T>
        //void operator=(T& val) { m_Component->SetProperty<T>(m_PropertyName, val); }

        // Specialization for string literals
        template<std::size_t N>
        void operator=(const char(&val)[N]) { m_Component->SetProperty<N>(m_PropertyName, val); }
    };
    SubscriptProxy operator[](std::string propertyName) { return SubscriptProxy(this, propertyName); }
};

// TODO: Move this to Tests once entity importing is finished
class ComponentWrapperFactory
{
public:
    ComponentWrapperFactory() = default;
    ComponentWrapperFactory(std::string componentTypeName, std::size_t allocation = 0)
    {
        m_ComponentInfo.Name = componentTypeName;
        m_ComponentInfo.Meta->Allocation = allocation;
    }

    template <typename T>
    void AddProperty(std::string fieldName, T defaultValue)
    {
        m_DefaultValues.push_back(defaultValue);
        m_ComponentInfo.Fields[fieldName].Name = typeid(T).name();
        m_ComponentInfo.Fields[fieldName].Offset = m_ComponentInfo.Stride;
        m_ComponentInfo.Fields[fieldName].Stride = sizeof(T);
        m_ComponentInfo.Stride += sizeof(T);
    }
    
    ComponentInfo& Finalize()
    {
        m_ComponentInfo.Defaults = std::shared_ptr<char>(new char[m_ComponentInfo.Stride]);
        std::size_t offset = 0;
        for (auto& val : m_DefaultValues) {
            memcpy(m_ComponentInfo.Defaults.get() + offset, val.Data.get(), val.Size);
            offset += val.Size;
        }

        return m_ComponentInfo;
    }

    operator ComponentInfo&() { return Finalize(); }

private:
    ComponentInfo m_ComponentInfo;
    std::vector<Any> m_DefaultValues;
};

#endif
