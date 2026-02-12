#include "PacketViolationSeverity.hpp"
#include "PacketViolationType.hpp"
#include "deps/ReadOnlyBinaryStream.hpp"
#include "deps/memory/Hook.hpp"

INSTANCE_HOOK(
    BatchedNetworkPeerSendPacketHook,
    memory::HookPriority::Normal,
    memory::resolveIdentifier(
        {// 1.21.50
         "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 70 41 8B F1 48 8B FA 48 8B D9 48 83 C1 18 48 83 7A ?? ?? 76 ?? "
         "48 8B 12",
         // 1.21.60 - 1.21.130
         "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 81 EC 80 00 00 00 41 8B F1 48 8B FA 48 8B D9 48 83 C1 18 48 8B 01 4C "
         "8B 90 ?? ?? ?? ?? 48 8B C2 48 83 7A ?? ?? 76 ?? 48 8B 02"
        }
    ),
    void,
    std::string& buffer,
    int          reliability,
    int          compressible
) {
    auto stream   = ReadOnlyBinaryStream(buffer);
    auto header   = stream.getUnsignedVarInt();
    auto packetId = header & 0x3FF;
    if (packetId == 156) { // MinecraftPacketIds::PacketViolationWarning
        auto type     = static_cast<PacketViolationType>(stream.getVarInt());
        auto severity = static_cast<PacketViolationSeverity>(stream.getVarInt());
        auto errorId  = stream.getVarInt();
        auto context  = stream.getString();
    }
    return origin(buffer, reliability, compressible);
}