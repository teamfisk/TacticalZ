#ifndef MessageType_h__
#define MessageType_h__

// Message types used by both server and client.
// Used to determine what type of message was sent.
enum class MessageType
{
	Connect,
	Disconnect,
	ClientPing,
	ServerPing,
	Message,
	Snapshot,
    OnInputCommand,
    OnPlayerDamage,
    PlayerConnected
};

#endif
