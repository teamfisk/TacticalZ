#include "..\..\..\include\Engine\Network\NetworkClient.h"

NetworkClient::NetworkClient()
{ 
    m_ReadBuffer = new char[m_BufferSize];
}

NetworkClient::~NetworkClient()
{ }
