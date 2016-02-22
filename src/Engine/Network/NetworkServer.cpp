#include "Network/NetworkServer.h"

NetworkServer::NetworkServer()
{
    m_ReadBuffer = new char[m_BufferSize];
}

NetworkServer::~NetworkServer()
{ 
    delete[] m_ReadBuffer;
}
