#ifndef ComponentWrapper_h__
#define ComponentWrapper_h__

#include <boost/shared_array.hpp>
#include "../Common.h"
#include "Entity.h"
#include "ComponentInfo.h"
#include "Util/Any.h"

template <typename T, typename Enable = void>
struct ComponentField { };

template <typename T>
struct ComponentField<T, typename std::enable_if<std::is_trivially_copyable<T>::value>::type>
{
    static T& Get(const ComponentInfo::Field_t& info, char* data) { return *reinterpret_cast<T*>(data); }
    static void Set(const ComponentInfo::Field_t& info, char* data, const T& value) { Get(data) = value; }
};

template <>
struct ComponentField<std::string, void>
{
    static std::string& Get(const ComponentInfo::Field_t& info, char* data) { return **reinterpret_cast<std::string**>(data); }
    static void Set(const ComponentInfo::Field_t& info, char* data, const std::string& value) { Get(info, data) = value; }
};

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

    ComponentInfo::EnumType Enum(const char* fieldName, const char* enumKey)
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

    void Copy(ComponentWrapper& destination)
    {
        // Copy trivial data
        memcpy(destination.Data, Data, Info.Stride);
        // Duplicate strings
        SolidifyStrings(destination);
    }

    // When component data has been copied, strings need to be reconstructed or they'll refer to the same data!
    static void SolidifyStrings(ComponentWrapper& component)
    {
        for (auto& name : component.Info.StringFields) {
            std::size_t offset = component.Info.Fields.at(name).Offset;
            auto& value = *reinterpret_cast<const std::string*>(component.Data + offset);
            new (component.Data + offset) std::string(value);
        }
    }
    
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
        ComponentInfo::EnumType Enum(const char* enumKey) { return m_Component->Enum(m_PropertyName.c_str(), enumKey); }

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

// A component wrapper that "owns" its data through a shared pointer
struct SharedComponentWrapper : ComponentWrapper
{
    SharedComponentWrapper(const ComponentInfo& componentInfo, boost::shared_array<char> data)
        : ComponentWrapper(componentInfo, data.get())
        , m_DataReference(data)
    { }

private:
    boost::shared_array<char> m_DataReference;
};

#endif
