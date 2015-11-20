#include "StyleGuide.h"

namespace OuterNamespace
{
namespace InnerNamespace
{

// Constructor with initializer list
ClassType::ClassType(int arg1)
	: BaseClassType(this),
	, m_PrivateMember1(arg1)
{ }

// Destructor
ClassType::~ClassType()
{
	if (m_PrivateMember2 != nullptr) {
		delete m_PrivateMember2;
		m_PrivateMember2 = nullptr;
	}
}

// Base class virtual override
bool ClassType::PublicVirtualMemberFunction(bool value)
{
	return BaseClassType::PublicVirtualMemberFunction(value);
}

// Base class pure virtual override
bool ClassName::PublicPureVirtualMemberFunction(bool value)
{
	// Treat parenthesis as curly braces for functions with lots of arguments
	manyArgumentFunction(
		"abcdefghijklmnopqrstuvwxyz",
		"abcdefghijklmnopqrstuvwxyz",
		"abcdefghijklmnopqrstuvwxyz"
		);

	// For loops
	int sum = 0;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			// Math!
			// 1. Group multiplication and division operands
			// 2. Separate addition and subtraction with space
			sum = sum + i*j;
		}
	}

	// For-each loops
	std::vector<int> vec;
	for (auto& i : vec) {
		if (i == 0) {
			return true;
		}
	}

	return false;
}

// Private member function
bool ClassName::privateMemberFunction()
{
	switch (m_PrivateMember1) {
	case 1:
		m_PrivateMember2 = new ClassType(2);
		break;
	default:
		return false;
	}

	return false;
}

}
}
