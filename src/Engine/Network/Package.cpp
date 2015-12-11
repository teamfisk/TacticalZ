#include "Network/Package.h"

Package::Package(MessageType type,unsigned int& packageID)
{
	// Create message header
	// Add message type
	int messageType = static_cast<int>(type);
    Package::AddPrimitive<int>(messageType);
    packageID = packageID % 1000; // Packet id modulos
    Package::AddPrimitive<int>(packageID);
    packageID++;
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
