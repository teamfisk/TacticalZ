#ifndef Namespace_FileName_h__
#define Namespace_FileName_h__

#include <standard_library>

#include <external_dependency>

// Relative paths preferred
#include "Internal/Header.h"

namespace OuterNamespace
{
namespace InnerNamespace
{

enum class EnumType
{
	Red,
	Green,
	Blue
};

struct StructType
{
	int PublicMember = 0;
};

template <typename T>
class BaseClassType
{
public:
	BaseClassType(T* child)
		: m_Child(child)
	{ }

	virtual bool PublicVirtualMemberFunction(bool value) { return value; }
	virtual bool PublicPureVirtualMemberFunction(bool value) = 0;

private:
	T* m_Child = nullptr;
};

class ClassType : public BaseClassType<ClassType>
{
public:
	// Constructor
	ClassType(int arg1);

	// Destructor
	~ClassType();

	// Getter
	int PrivateMember1() const { return m_PrivateMember1; } // Prefer copy for fundamental types
	const StructType* PrivateMember2() const { return m_PrivateMember2; }
	// Setter
	void SetPrivateMember1(const int& privateMember1) { m_PrivateMember1 = privateMember1; }
	void SetPrivateMember2(StructType* privateMember2) { m_PrivateMember2 = privateMember2; }

	// Public templated member function
	template <typename T>
	T* PublicMemberFunction(bool value);

	// Base class virtual override
	bool PublicVirtualMemberFunction(bool value) override;

	// Base class pure virtual override
	bool PublicPureVirtualMemberFunction(bool value) override;

private:
	// Private members with default values
	int m_PrivateMember1 = 1;
	StructType* m_PrivateMember2 = nullptr;

	// Private member functions
	bool privateMemberFunction();
	void manyArgumentFunction(std::string arg1, std::string arg2, std::string arg3) { }
};

// Public templated member function
template <typename T>
T* ClassType::PublicMemberFunction(bool value)
{
	if (m_PrivateMember2 == nullptr) {
		return nullptr;
	}

	if (m_PrivateMember2->PublicMember == 1) {
		return new T();
	}
	else {
		return nullptr;
	}
}

}
}

#endif