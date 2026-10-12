#pragma once

#include "channel/channel_registry.hpp"
#include "channel/script_store.hpp"

#include <string>
#include <vector>

namespace dnf { namespace channel {

struct BridgeIdentity {
    int id;
    std::string advertised_ip;
    int udp_port;
};

struct BridgeHandleResult {
    BridgeHandleResult() : accepted(false), gc_ready(false) {}
    bool accepted;
    bool gc_ready;
    std::vector<Bytes> outbound;
    std::string error;
};

class BridgeProtocolClient {
public:
    BridgeProtocolClient(ScriptStore& scripts, ChannelRegistry& registry, BridgeIdentity identity)
        : scripts_(scripts), registry_(registry), identity_(identity), connection_gc_ready_(false) {}

    std::vector<Bytes> BuildInitialFrames() const;
    BridgeHandleResult HandleFrame(const Bytes& frame);
    void Reset();

private:
    Bytes BuildGetGcInfo() const;
    Bytes BuildCheckScriptVersion() const;
    Bytes BuildNotice() const;
    Bytes BuildGetScript() const;

    ScriptStore& scripts_;
    ChannelRegistry& registry_;
    BridgeIdentity identity_;
    std::string pending_version_;
    bool connection_gc_ready_;
};

// Thin POSIX transport. Reconnect policy deliberately stays outside the wire
// core so the existing daemon supervisor can retain ownership of lifecycle.
class PosixBridgeConnection {
public:
    PosixBridgeConnection();
    ~PosixBridgeConnection();
    bool Connect(const std::string& host, int port, std::string* error = nullptr);
    bool Send(const Bytes& frame, std::string* error = nullptr);
    bool Receive(Bytes* frame, std::string* error = nullptr);
    void Close();

private:
    int fd_;
};

} }  // namespace dnf::channel
