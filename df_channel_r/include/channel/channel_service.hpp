#pragma once

#include "channel/channel_registry.hpp"
#include "channel/legacy_codec.hpp"
#include "channel/script_store.hpp"

#include <string>
#include <utility>

namespace dnf { namespace channel {

struct HandleResult {
    HandleResult(bool ok, bool has, const Bytes& bytes, const std::string& message)
        : accepted(ok), has_response(has), response(bytes), error(message) {}
    HandleResult(bool ok, bool has, Bytes&& bytes, const std::string& message)
        : accepted(ok), has_response(has), response(std::move(bytes)), error(message) {}
    bool accepted;
    bool has_response;
    Bytes response;
    std::string error;
};

class ChannelService {
public:
    ChannelService(ScriptStore& scripts, ChannelRegistry& registry, LegacyCodec& codec)
        : scripts_(scripts), registry_(registry), codec_(codec) {}

    HandleResult HandleGameFrame(const Bytes& frame);

private:
    HandleResult HandleConnect(const Bytes& frame);
    HandleResult HandleCheckScriptVersion(const Bytes& frame);
    HandleResult HandleGetScript();
    HandleResult HandleAskChannelInfo();
    HandleResult HandleGetServerGroupChannelInfo(const Bytes& frame);
    HandleResult HandleUpdateChannelInfo(const Bytes& frame);

    ScriptStore& scripts_;
    ChannelRegistry& registry_;
    LegacyCodec& codec_;
};

// One instance per accepted client socket.  The old client may accept the key
// from a CONNECT reply with is_valid_key=0 and immediately request the script;
// the original dispatcher therefore did not gate other message IDs on a
// second CONNECT.  authenticated_ records an optional valid-key confirmation
// but is deliberately not an authorization boundary.
class ChannelSession {
public:
    explicit ChannelSession(ChannelService& service)
        : service_(service), authenticated_(false) {}

    HandleResult HandleGameFrame(const Bytes& frame);
    bool authenticated() const { return authenticated_; }

private:
    ChannelService& service_;
    bool authenticated_;
};

} }  // namespace dnf::channel
