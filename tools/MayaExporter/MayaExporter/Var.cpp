#include "Var.h"
#include "MayaIncludes.h"
#include <assert.h>

Var::Var(Var::Type type, unsigned int numElements)
{
	m_Type = type;
	m_NumElements = numElements;

	switch (m_Type)
	{
	case Var::Type::Int:
		m_Data = new int[m_NumElements];
		break;
	case Var::Type::Float:
		m_Data = new float[m_NumElements];
		break;
	case Var::Type::String:
		m_Data = new std::string[m_NumElements];
		break;
	default:
		MGlobal::displayError("Var is an invalid type");
		assert(0);
		break;
	}
}

Var& Var::operator=(const int& other)
{
	assert(m_Type == Type::Int);
	*(int*)m_Data = other;
	return *this;
}

Var& Var::operator=(const float& other)
{
	assert(m_Type == Type::Float);
	*(float*)m_Data = other;
	return *this;
}

Var& Var::operator=(const std::string& other)
{
	assert(m_Type == Type::String);
	std::string tmp(other);
	(*(std::string*)m_Data).swap(tmp);
	return *this;
}

//Var& Var::operator=(const Var& other)
//{
//	m_Data = other.m_Data;
//	m_Type = other.m_Type;
//	return *this;
//}

Var::Type Var::isType() const
{
	return m_Type;
}

unsigned int Var::NumElements() const
{
	return m_NumElements;
}

void* Var::Data()
{
	return m_Data;
}