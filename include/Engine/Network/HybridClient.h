#ifndef HybridClient_h__
#define HybridClient_h__

#include "Client.h"


class HybridClient : public Client
{
public:
    HybridClient(ConfigFile* config);
    ~HybridClient();
};

#endif