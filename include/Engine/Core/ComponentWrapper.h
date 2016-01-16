#ifndef ComponentWrapper_h__
#define ComponentWrapper_h__

#include "../Common.h"
#include "Entity.h"
#include "ComponentInfo.h"
#include "Util/Any.h"

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
    T& Field(std::string name)
    {
        const ComponentInfo::Field_t& field = Info.Fields.at(name);
        if (sizeof(T) > field.Stride) {
            std::stringstream message;
            message << "Type size of \"" << typeid(T).name() << "\" doesn't match size of component field \"" << Info.Name << "." << name << "\"!";
            throw new std::runtime_error(message.str().c_str());
        } 
        return *reinterpret_cast<T*>(&Data[field.Offset]);
    }

    template <typename T>
    void SetField(std::string name, const T value) { Field<T>(name) = value; }
    //template <typename T>
    //void SetField(std::string name, T& value) { Field<T>(name) = value; }

    // Specialization for string literals
    template <std::size_t N>
    void SetField(std::string name, const char(&value)[N]) { Field<std::string>(name) = std::string(value); }
    
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
        operator T&() { return m_Component->Field<T>(m_PropertyName); }

        template <typename T>
        void operator=(const T val) { m_Component->SetField<T>(m_PropertyName, val); }
        // TODO: Pass by reference and rvalue (universal reference?)
        //template <typename T>
        //void operator=(T& val) { m_Component->SetField<T>(m_PropertyName, val); }

        // Specialization for string literals
        template <std::size_t N>
        void operator=(const char(&val)[N]) { m_Component->SetField<N>(m_PropertyName, val); }
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
