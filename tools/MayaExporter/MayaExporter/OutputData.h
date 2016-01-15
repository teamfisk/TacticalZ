#ifndef OutputData_OutputData_h__
#define OutputData_OutputData_h__
#include <fstream>
//template <typename T>
class OutputData
{
public://std::ostream& out, const OutputData& obj
	//OutputData(T& object)
	//	: m_Object(object)
	//{};

	friend std::ostream& operator<<(std::ostream& out, const OutputData& obj)
	{
		obj.WriteASCII(out);
		return out;
	};
	virtual void WriteBinary(std::ostream& out) = 0;
	virtual void WriteASCII(std::ostream& out) const = 0;

	//T& m_Object = nullptr;
};

#endif