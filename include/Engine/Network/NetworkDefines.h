#ifndef MessageType_h__
#define MessageType_h__

#include <boost/asio.hpp>
#include <queue>

#define BOARDSIZE 16
#define MAXCONNECTIONS 8
#define INPUTSIZE 128

typedef boost::shared_ptr<boost::asio::ip::udp::socket> socket_ptr;
typedef boost::shared_ptr<std::string> string_ptr;
typedef boost::shared_ptr<std::queue<string_ptr>> messageQueue_ptr;

#endif