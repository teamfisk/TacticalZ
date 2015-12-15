#ifndef Var_Var_h__
#define Var_Var_h__

#include <string>

class Var
{
public:
	enum class Type
	{
		Int,
		Float,
		String
	};

	Var(Var::Type type, unsigned int numElements = 1);
	~Var() { delete[] m_Data; };

	Var& operator=(const int& other);
	Var& operator=(const float& other);
	Var& operator=(const std::string& other);
	//Var& operator=(const Var& other);
	Type isType() const;
	unsigned int NumElements() const;
	void* Data();
private:
	Type m_Type;
	unsigned int m_NumElements;
	void* m_Data;
};

#endif