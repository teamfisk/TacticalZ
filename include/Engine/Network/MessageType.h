#ifndef MessageType_h__
#define MessageType_h__

// Message types used by both server and client.
// Used to determine what type of message was sent.
enum class MessageType
{
    Connect,
    Disconnect,
    Ping,
    Message,
    Snapshot,
    OnInputCommand,
    OnPlayerDamage,
    PlayerConnected,
    BecomePlayer,
    Kick,
    OnPlayerSpawned,
    EntityDeleted,
    ComponentDeleted,
    PlayerTransform,
    OnDoubleJump,
    OnDashEffect,
    ServerlistRequest,
    AmmoPickup,
    RemoveWorld,
    Invalid
};

#endif
