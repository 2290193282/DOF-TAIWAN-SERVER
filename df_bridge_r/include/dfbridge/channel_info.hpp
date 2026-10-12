#pragma once

#include "dfbridge/channel_model.hpp"

#include <string>

namespace dfbridge {

// Recreates ChannelServiceApp::ScriptThread's database-to-file projection.
class ChannelInfoWriter {
public:
    std::string Render(const ChannelSnapshot& snapshot) const;
    void WriteAtomically(const ChannelSnapshot& snapshot,
                         const std::string& target) const;
};

// Audit/test parser. It accepts both forms seen in official data:
//   [server] 17
// and the bridge-generated legacy form:
//   [server]\n17
class ChannelInfoParser {
public:
    ChannelSnapshot Parse(const std::string& text) const;
    ChannelSnapshot ParseFile(const std::string& path) const;
};

std::string NormalizeKindName(std::string value);
std::string RenderDungeonKind(std::string value);
std::string RenderServerKind(std::string value);
std::string ReadChannelInfoFile(const std::string& path);
bool ChannelSnapshotsSemanticallyEquivalent(const ChannelSnapshot& lhs,
                                            const ChannelSnapshot& rhs);

}  // namespace dfbridge
