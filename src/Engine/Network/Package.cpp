#include "Network/Package.h"

Package::Package(MessageType type)
{
	// Create message header
	// Add message type
	int messageType = static_cast<int>(type);
	memcpy(m_Data, &messageType, sizeof(int));
	m_Offset += sizeof(int);
}

Package::~Package()
{
	delete[] m_Data;
}



void Package::AddString(std::string str)
{
	// Message, add one extra byte for null terminator
	memcpy(m_Data + m_Offset, str.data(), (str.size() + 1) * sizeof(char));
	m_Offset += (str.size() + 1) * sizeof(char);
}
