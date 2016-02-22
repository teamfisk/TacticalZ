#include "Network/NetworkClient.h"

NetworkClient::NetworkClient()
{ 
    m_ReadBuffer = new char[m_BufferSize];
}

NetworkClient::~NetworkClient()
{ 
    delete[] m_ReadBuffer;
}
