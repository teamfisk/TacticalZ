#ifndef ComponentWrapper_h__
#define ComponentWrapper_h__

#include <boost/shared_array.hpp>
#include <boost/any.hpp>
#include "../Common.h"
#include "../GLM.h"
#include "Entity.h"
#include "ComponentInfo.h"
#include "DirtySet.h"
#include "Util/Any.h"


struct ComponentWrapper
{
    ComponentWrapper(const ComponentInfo& componentInfo, char* data, ::DirtyBitField* dirtyBitField)
        : Info(componentInfo)
        , EntityID(*reinterpret_cast<::EntityID*>(data))
        , Data(data + componentInfo.GetHeaderSize())
        , DirtyBitField(dirtyBitField)
    { }

    const ComponentInfo& Info;
    const ::EntityID EntityID;
    char* Data;
    ::DirtyBitField* DirtyBitField = nullptr;

    ComponentInfo::EnumType Enum(const char* fieldName, const char* enumKey)
    {
        return Info.Meta->FieldEnumDefinitions.at(fieldName).at(enumKey);
    }

    bool Dirty(DirtySetType type, const std::string& fieldName)
    {
        if (DirtyBitField == nullptr) {
            return true;
        } else {
            auto& field = Info.Fields.at(fieldName);
            return DirtyBitField->operator[](type).count(field.Index) == 1;
        }
    }

    void SetDirty(DirtySetType type, const std::string& fieldName, bool dirty = true)
    {
        if (DirtyBitField == nullptr) {
            return;
        }

        auto& field = Info.Fields.at(fieldName);
        if (dirty) {
            DirtyBitField->operator[](type).insert(field.Index);
        } else {
            DirtyBitField->operator[](type).erase(field.Index);
        }
    }

    template <typename T>
    T& Field(const std::string& name)
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
    void SetField(const std::string& name, const T value) { Field<T>(name) = value; }
    //template <typename T>
    //void SetField(std::string name, T& value) { Field<T>(name) = value; }

    // Specialization for string literals
    template <std::size_t N>
    void SetField(const std::string& name, const char(&value)[N]) { Field<std::string>(name) = std::string(value); }

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
            std::string value = *reinterpret_cast<const std::string*>(component.Data + offset);
            new (component.Data + offset) std::string(value);
        }
    }

    // This needs to be called to properly free component data, because strings.
    static void Destroy(ComponentInfo info, char* data)
    {
        // Call std::string destructors
        for (auto& name : info.StringFields) {
            std::size_t offset = info.Fields.at(name).Offset;
            auto field = reinterpret_cast<std::string*>(data + offset);
            field->~basic_string();
        }
    }
    
    struct SubscriptProxy
    {
        friend struct ComponentWrapper;
  
    public:
        SubscriptProxy(ComponentWrapper* component, std::string fieldName)
            : m_Component(component)
            , m_FieldName(fieldName)
        { }

        ComponentWrapper* m_Component;
        std::string m_FieldName;

    public:
        // Return the integer value of an enum type key for this field
        ComponentInfo::EnumType Enum(const char* enumKey) { return m_Component->Enum(m_FieldName.c_str(), enumKey); }
        bool Dirty(DirtySetType type) { return m_Component->Dirty(type, m_FieldName); }
        void SetDirty(DirtySetType type, bool dirty = true) { m_Component->SetDirty(type, m_FieldName, dirty); }

        //template <
        //    typename T,
        //    typename = typename std::enable_if<!std::is_base_of<::FIELDLOL, T>::value>::type
        //>
       // operator const T&() { return m_Component->Field<T>(m_FieldName); }
        operator const double&() { return m_Component->Field<double>(m_FieldName); }
        operator const float&() { return m_Component->Field<float>(m_FieldName); }
        operator const int&() { return m_Component->Field<int>(m_FieldName); }
        operator const glm::vec3&() { return m_Component->Field<glm::vec3>(m_FieldName); }
        operator const glm::vec4&() { return m_Component->Field<glm::vec4>(m_FieldName); }
        operator const glm::quat&() { return m_Component->Field<glm::quat>(m_FieldName); }
        operator const bool&() { return m_Component->Field<bool>(m_FieldName); }
        operator const std::string&() { return m_Component->Field<std::string>(m_FieldName); }

        template <
            typename T,
            typename = typename std::enable_if<!std::is_base_of<::FIELDLOL, T>::value>::type
        >
        //operator T&() { static_assert(constexpr(false), "FU"); }
        operator T&() = delete;

       /* template <
            typename T,
            typename = typename std::enable_if<!std::is_base_of<::FIELDLOL, T>::value>::type
        >
        operator T&() { static_assert(constexpr(false), "FU"); }*/

        //template <typename T>
        //operator typename std::enable_if<!std::is_base_of<::FIELDLOL, T>::value, T>::type() { return m_Component->Field<T>(m_FieldName); }

        //template <typename T>
        //operator Field<T>() 
        //{
        //    return ::Field<T>(m_Component->Field<T>(m_FieldName)); 
        //}

        //operator std::string() { return m_Component->Field<std::string>(m_FieldName); }
        //operator glm::vec3() { return m_Component->Field<glm::vec3>(m_FieldName); }

        //template <typename T>
        //operator glm::vec3() { return m_Component->Field<glm::vec3>(m_FieldName); }

        //template <typename T>
        //operator const T&() { return m_Component->Field<T>(m_FieldName); }

        template <typename T>
        void operator=(const T& val) { m_Component->SetField<T>(m_FieldName, val); }
        // TODO: Pass by reference and rvalue (universal reference?)
        //template <typename T>
        //void operator=(T& val) { m_Component->SetField<T>(m_PropertyName, val); }

        // Specialization for string literals
        template <std::size_t N>
        void operator=(const char(&val)[N]) { m_Component->SetField<N>(m_FieldName, val); }
    };
    SubscriptProxy operator[](const std::string& propertyName) { return SubscriptProxy(this, propertyName); }
};

struct FIELDLOL {};

template <typename T>
struct FieldBase : FIELDLOL
{
    FieldBase(ComponentWrapper::SubscriptProxy& proxy)
        : Data(proxy.m_Component->Field<T>(proxy.m_FieldName))
    { }

    FieldBase& operator=(const FieldBase& rhs) { Data = rhs.Data; return *this; }
    FieldBase& operator=(const T& rhs) { Data = rhs; return *this; }

    template <typename T2> FieldBase& operator+=(const T2& rhs) { Data += rhs; return *this; }
    template <typename T2> FieldBase& operator-=(const T2& rhs) { Data -= rhs; return *this; }
    template <typename T2> FieldBase& operator*=(const T2& rhs) { Data *= rhs; return *this; }
    template <typename T2> FieldBase& operator/=(const T2& rhs) { Data /= rhs; return *this; }
    template <typename T2> FieldBase& operator%=(const T2& rhs) { Data %= rhs; return *this; }
    template <typename T2> FieldBase& operator&=(const T2& rhs) { Data &= rhs; return *this; }
    template <typename T2> FieldBase& operator|=(const T2& rhs) { Data |= rhs; return *this; }
    template <typename T2> FieldBase& operator^=(const T2& rhs) { Data ^= rhs; return *this; }
    template <typename T2> FieldBase& operator<<=(const T2& rhs) { Data <<= rhs; return *this; }
    template <typename T2> FieldBase& operator>>=(const T2& rhs) { Data >>= rhs; return *this; }

    T operator+() const { return +Data; }
    T operator-() const { return -Data; }
    T operator~() const { return ~Data; }

    FieldBase& operator++() { Data++; return *this; }
    T operator++(int) { T tmp = Data; operator++(); return tmp; }
    FieldBase& operator--() { Data--; return *this; }
    T operator--(int) { T tmp = Data; operator--(); return tmp; }

    operator const T&() const { return Data; }
    const T& operator*() const { return operator const T&(); }

protected:
    T& Data;
};

template <typename T>
struct Field : FieldBase<T>
{
    //Field(T& Data)
    //    : FieldBase(Data)
    //{ }
    using FieldBase<T>::FieldBase;
    using FieldBase<T>::operator=;
};

template <>
struct Field<glm::vec3> : FieldBase<glm::vec3>
{
    //Field(glm::vec3& Data)
    //    : FieldBase(Data)
    //{ }
    using FieldBase<glm::vec3>::FieldBase;
    using FieldBase<glm::vec3>::operator=;

    glm::vec3::value_type x() { return Data.x; }
    void x(glm::vec3::value_type val) { Data.x = val; }
    glm::vec3::value_type y() { return Data.y; }
    void y(glm::vec3::value_type val) { Data.y = val; }
    glm::vec3::value_type z() { return Data.z; }
    void z(glm::vec3::value_type val) { Data.z = val; }

    template <typename T> friend glm::vec3 operator+(const Field<glm::vec3>& lhs, const T& rhs) { return static_cast<glm::vec3>(lhs) + glm::vec3(rhs); }
    template <typename T> friend glm::vec3 operator-(const Field<glm::vec3>& lhs, const T& rhs) { return static_cast<glm::vec3>(lhs) - glm::vec3(rhs); }
    template <typename T> friend glm::vec3 operator*(const Field<glm::vec3>& lhs, const T& rhs) { return static_cast<glm::vec3>(lhs) * glm::vec3(rhs); }
    template <typename T> friend glm::vec3 operator/(const Field<glm::vec3>& lhs, const T& rhs) { return static_cast<glm::vec3>(lhs) / glm::vec3(rhs); }
    template <typename T> friend glm::vec3 operator+(const T& lhs, const Field<glm::vec3>& rhs) { return glm::vec3(lhs) + static_cast<glm::vec3>(rhs); }
    template <typename T> friend glm::vec3 operator-(const T& lhs, const Field<glm::vec3>& rhs) { return glm::vec3(lhs) - static_cast<glm::vec3>(rhs); }
    template <typename T> friend glm::vec3 operator*(const T& lhs, const Field<glm::vec3>& rhs) { return glm::vec3(lhs) * static_cast<glm::vec3>(rhs); }
    template <typename T> friend glm::vec3 operator/(const T& lhs, const Field<glm::vec3>& rhs) { return glm::vec3(lhs) / static_cast<glm::vec3>(rhs); }

    friend glm::vec3& operator+=(glm::vec3& lhs, const Field<glm::vec3>& rhs) { lhs += *rhs; return lhs; }
};

template <>
struct Field<glm::vec4> : FieldBase<glm::vec4>
{
    //Field(glm::vec4& Data)
    //    : FieldBase(Data)
    //{ }
    using FieldBase<glm::vec4>::FieldBase;
    using FieldBase<glm::vec4>::operator=;

    glm::vec4::value_type x() { return Data.x; }
    void x(glm::vec4::value_type val) { Data.x = val; }
    glm::vec4::value_type y() { return Data.y; }
    void y(glm::vec4::value_type val) { Data.y = val; }
    glm::vec4::value_type z() { return Data.z; }
    void z(glm::vec4::value_type val) { Data.z = val; }
    glm::vec4::value_type w() { return Data.w; }
    void w(glm::vec4::value_type val) { Data.w = val; }

    template <typename T> friend glm::vec4 operator+(const Field<glm::vec4>& lhs, const T& rhs) { return static_cast<glm::vec4>(lhs) + glm::vec4(rhs); }
    template <typename T> friend glm::vec4 operator-(const Field<glm::vec4>& lhs, const T& rhs) { return static_cast<glm::vec4>(lhs) - glm::vec4(rhs); }
    template <typename T> friend glm::vec4 operator*(const Field<glm::vec4>& lhs, const T& rhs) { return static_cast<glm::vec4>(lhs) * glm::vec4(rhs); }
    template <typename T> friend glm::vec4 operator/(const Field<glm::vec4>& lhs, const T& rhs) { return static_cast<glm::vec4>(lhs) / glm::vec4(rhs); }
    template <typename T> friend glm::vec4 operator+(const T& lhs, const Field<glm::vec4>& rhs) { return glm::vec4(lhs) + static_cast<glm::vec4>(rhs); }
    template <typename T> friend glm::vec4 operator-(const T& lhs, const Field<glm::vec4>& rhs) { return glm::vec4(lhs) - static_cast<glm::vec4>(rhs); }
    template <typename T> friend glm::vec4 operator*(const T& lhs, const Field<glm::vec4>& rhs) { return glm::vec4(lhs) * static_cast<glm::vec4>(rhs); }
    template <typename T> friend glm::vec4 operator/(const T& lhs, const Field<glm::vec4>& rhs) { return glm::vec4(lhs) / static_cast<glm::vec4>(rhs); }

    friend glm::vec4& operator+=(glm::vec4& lhs, const Field<glm::vec4>& rhs) { lhs += *rhs; return lhs; }
};


// A component wrapper that "owns" its data through a shared pointer
struct SharedComponentWrapper : ComponentWrapper
{
    SharedComponentWrapper(const ComponentInfo& componentInfo, boost::shared_array<char> data)
        : ComponentWrapper(componentInfo, data.get(), nullptr)
        , m_DataReference(data)
    { }

private:
    boost::shared_array<char> m_DataReference;
};

// TODO: Move this to Tests once entity importing is finished
class ComponentWrapperFactory
{
public:
    ComponentWrapperFactory() = default;
    ComponentWrapperFactory(std::string componentTypeName, unsigned int allocation = 0)
    {
        m_ComponentInfo.Name = componentTypeName;
        m_ComponentInfo.Meta = std::make_shared<ComponentInfo::Meta_t>();
        m_ComponentInfo.Meta->Allocation = allocation;
    }

    template <typename T>
    void AddProperty(std::string fieldName, T defaultValue)
    {
        auto& field = m_ComponentInfo.Fields[fieldName];
        field.Name = fieldName;
        field.Type = typeid(T).name();
        field.Offset = m_ComponentInfo.Stride;
        field.Stride = sizeof(T);
        m_ComponentInfo.FieldsInOrder.push_back(field.Name);
        if (field.Type == typeid(std::string).name()) {
            field.Type = "string";
            m_ComponentInfo.StringFields.push_back(field.Name);
        }
        m_ComponentInfo.Stride += sizeof(T);
        m_DefaultValues.push_back(std::make_pair(field, defaultValue));
    }

    ComponentInfo& Finalize()
    {
        m_ComponentInfo.Defaults = boost::shared_array<char>(new char[m_ComponentInfo.Stride]);
        std::size_t offset = 0;
        for (auto& pair : m_DefaultValues) {
            if (pair.first.Type == "string") {
                new (m_ComponentInfo.Defaults.get() + offset) std::string(*reinterpret_cast<std::string*>(pair.second.Data.get()));
            } else {
                memcpy(m_ComponentInfo.Defaults.get() + offset, pair.second.Data.get(), pair.second.Size);
            }
            offset += pair.second.Size;
        }

        return m_ComponentInfo;
    }

    operator ComponentInfo&() { return Finalize(); }

private:
    ComponentInfo m_ComponentInfo;
    std::vector<std::pair<ComponentInfo::Field_t, Any>> m_DefaultValues;
};

#endif
