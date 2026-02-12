#pragma once

enum class PacketViolationSeverity : int {
    Unknown               = -1,
    Warning               = 0,
    FinalWarning          = 1,
    TerminatingConnection = 2,
};