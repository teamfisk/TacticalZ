#ifndef NetworkDefines_h__
#define NetworkDefines_h__

#include <boost/asio/ip/udp.hpp>
#include <queue>
#include "Network/Package.h"


#define BOARDSIZE 16
#define MAXCONNECTIONS 8
#define INPUTSIZE 128
#define PLAYERSPEED 0.2f;

typedef boost::shared_ptr<boost::asio::ip::udp::socket> socket_ptr;
typedef boost::shared_ptr<std::string> string_ptr;
typedef boost::shared_ptr<std::queue<string_ptr>> messageQueue_ptr;



#endif