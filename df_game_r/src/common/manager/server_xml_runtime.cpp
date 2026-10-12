#include "common/manager/server_xml_runtime.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <vector>

// The original binary uses TinyXML.  The source tree has no TinyXML runtime,
// so this is a small XML tree reader for the exact server_str.xml grammar.  It
// keeps the original load/validate/store lifecycle instead of replacing the
// resource with a lookup-time default map.
class TiXmlNode {
public:
    std::string name;
    std::map<std::string, std::string> attributes;
    std::vector<TiXmlNode> children;
};

namespace {

bool StartsWith(const std::string& input, size_t offset, const char* text) {
    const std::string needle(text);
    return offset + needle.size() <= input.size() &&
           input.compare(offset, needle.size(), needle) == 0;
}

std::string DecodeEntities(const std::string& value) {
    std::string decoded;
    decoded.reserve(value.size());
    for (size_t index = 0; index < value.size(); ++index) {
        if (value[index] != '&') {
            decoded.push_back(value[index]);
            continue;
        }
        if (value.compare(index, 5, "&amp;") == 0) {
            decoded.push_back('&');
            index += 4;
        } else if (value.compare(index, 4, "&lt;") == 0) {
            decoded.push_back('<');
            index += 3;
        } else if (value.compare(index, 4, "&gt;") == 0) {
            decoded.push_back('>');
            index += 3;
        } else if (value.compare(index, 6, "&quot;") == 0) {
            decoded.push_back('"');
            index += 5;
        } else if (value.compare(index, 6, "&apos;") == 0) {
            decoded.push_back('\'');
            index += 5;
        } else {
            decoded.push_back('&');
        }
    }
    return decoded;
}

class ServerXmlReader {
public:
    explicit ServerXmlReader(const std::string& input) : input_(input), pos_(0) {}

    bool Parse(TiXmlNode* document) {
        while (true) {
            SkipWhitespace();
            if (pos_ == input_.size()) {
                return !document->children.empty();
            }
            if (SkipMarkup()) {
                continue;
            }
            TiXmlNode child;
            if (!ParseElement(&child)) {
                return false;
            }
            document->children.push_back(child);
        }
    }

private:
    bool IsNameChar(char value) const {
        return std::isalnum(static_cast<unsigned char>(value)) || value == '_' ||
               value == '-' || value == ':' || value == '.';
    }

    void SkipWhitespace() {
        while (pos_ < input_.size() &&
               std::isspace(static_cast<unsigned char>(input_[pos_]))) {
            ++pos_;
        }
    }

    bool SkipMarkup() {
        if (StartsWith(input_, pos_, "<?")) {
            const size_t end = input_.find("?>", pos_ + 2);
            if (end == std::string::npos) {
                pos_ = input_.size();
                return false;
            }
            pos_ = end + 2;
            return true;
        }
        if (StartsWith(input_, pos_, "<!--")) {
            const size_t end = input_.find("-->", pos_ + 4);
            if (end == std::string::npos) {
                pos_ = input_.size();
                return false;
            }
            pos_ = end + 3;
            return true;
        }
        if (StartsWith(input_, pos_, "<![CDATA[")) {
            return false;
        }
        if (StartsWith(input_, pos_, "<!")) {
            const size_t end = input_.find('>', pos_ + 2);
            if (end == std::string::npos) {
                pos_ = input_.size();
                return false;
            }
            pos_ = end + 1;
            return true;
        }
        return false;
    }

    bool ParseName(std::string* name) {
        const size_t begin = pos_;
        while (pos_ < input_.size() && IsNameChar(input_[pos_])) {
            ++pos_;
        }
        if (begin == pos_) {
            return false;
        }
        *name = input_.substr(begin, pos_ - begin);
        return true;
    }

    bool ParseAttributeValue(std::string* value) {
        if (pos_ == input_.size() ||
            (input_[pos_] != '\'' && input_[pos_] != '"')) {
            return false;
        }
        const char quote = input_[pos_++];
        const size_t begin = pos_;
        const size_t end = input_.find(quote, begin);
        if (end == std::string::npos) {
            return false;
        }
        *value = DecodeEntities(input_.substr(begin, end - begin));
        pos_ = end + 1;
        return true;
    }

    bool ParseElement(TiXmlNode* node) {
        if (pos_ == input_.size() || input_[pos_] != '<' ||
            StartsWith(input_, pos_, "</")) {
            return false;
        }
        ++pos_;
        if (!ParseName(&node->name)) {
            return false;
        }

        while (true) {
            SkipWhitespace();
            if (pos_ == input_.size()) {
                return false;
            }
            if (StartsWith(input_, pos_, "/>")) {
                pos_ += 2;
                return true;
            }
            if (input_[pos_] == '>') {
                ++pos_;
                break;
            }
            std::string key;
            std::string value;
            if (!ParseName(&key)) {
                return false;
            }
            SkipWhitespace();
            if (pos_ == input_.size() || input_[pos_] != '=') {
                return false;
            }
            ++pos_;
            SkipWhitespace();
            if (!ParseAttributeValue(&value)) {
                return false;
            }
            node->attributes[key] = value;
        }

        while (true) {
            if (pos_ == input_.size()) {
                return false;
            }
            if (StartsWith(input_, pos_, "</")) {
                pos_ += 2;
                std::string closing_name;
                if (!ParseName(&closing_name) || closing_name != node->name) {
                    return false;
                }
                SkipWhitespace();
                if (pos_ == input_.size() || input_[pos_] != '>') {
                    return false;
                }
                ++pos_;
                return true;
            }
            if (StartsWith(input_, pos_, "<!--")) {
                if (!SkipMarkup()) {
                    return false;
                }
                continue;
            }
            if (StartsWith(input_, pos_, "<![CDATA[")) {
                const size_t end = input_.find("]]>", pos_ + 9);
                if (end == std::string::npos) {
                    return false;
                }
                pos_ = end + 3;
                continue;
            }
            if (input_[pos_] == '<') {
                TiXmlNode child;
                if (!ParseElement(&child)) {
                    return false;
                }
                node->children.push_back(child);
                continue;
            }
            ++pos_;
        }
    }

    const std::string& input_;
    size_t pos_;
};

TiXmlNode* FirstChild(TiXmlNode* node, const char* name) {
    if (!node) {
        return 0;
    }
    for (std::vector<TiXmlNode>::iterator it = node->children.begin();
         it != node->children.end(); ++it) {
        if (it->name == name) {
            return &*it;
        }
    }
    return 0;
}

const char* Attribute(const TiXmlNode* node, const char* name) {
    if (!node) {
        return 0;
    }
    const std::map<std::string, std::string>::const_iterator it =
        node->attributes.find(name);
    return it == node->attributes.end() ? 0 : it->second.c_str();
}

int AttributeInt(const TiXmlNode* node, const char* name) {
    const char* value = Attribute(node, name);
    return value ? static_cast<int>(std::strtol(value, 0, 10)) : 0;
}

void TagError(const char* tag) {
    std::printf("[CServerXml] <%s> Tag Error\n", tag);
    std::exit(-1);
}

}  // namespace

namespace np_server_xml {

// IDA_EA: 0x80C5A14
// IDA_NAME: _ZN13np_server_xml10CServerXmlC2Ev
CServerXml::CServerXml()
    : charset_(),
      source_file_(),
      strings_(),
      event_start_strings_(),
      event_end_strings_(),
      event_rgba_() {
    InitString();
}

// IDA_EA: 0x80C5B26
// IDA_NAME: _ZN13np_server_xml10CServerXmlD2Ev
CServerXml::~CServerXml() {
    InitString();
}

// IDA_EA: 0x80C5C3E
// IDA_NAME: _ZN13np_server_xml10CServerXml10InitStringEv
int CServerXml::InitString() {
    charset_.clear();
    strings_.clear();
    event_start_strings_.clear();
    event_end_strings_.clear();
    event_rgba_.clear();
    return 0;
}

// IDA_EA: 0x80C5CA8
// IDA_NAME: _ZN13np_server_xml10CServerXml10StrLoadingEv
int CServerXml::StrLoading() {
    return StrLoading("server_str.xml");
}

// IDA_EA: 0x80C5D38
// IDA_NAME: _ZN13np_server_xml10CServerXml10StrLoadingESs
int CServerXml::StrLoading(std::string file_name) {
    InitString();
    source_file_ = file_name;

    std::ifstream input(file_name.c_str(), std::ios::in | std::ios::binary);
    if (!input) {
        return std::printf("[CServerXml] Load Fail File : %s\n",
                           file_name.c_str());
    }
    const std::string content((std::istreambuf_iterator<char>(input)),
                              std::istreambuf_iterator<char>());
    TiXmlNode document;
    ServerXmlReader reader(content);
    if (!reader.Parse(&document)) {
        return std::printf("[CServerXml] Load Fail File : %s\n",
                           file_name.c_str());
    }
    TiXmlNode* const root = FirstChild(&document, "xml");
    if (!root) {
        return std::puts("[CServerXml] <xml> Tag Error");
    }
    CharsetInit(root);
    EventLoad(root);
    return ProcessLoad(root);
}

// IDA_EA: 0x80C5E0E
// IDA_NAME: _ZN13np_server_xml10CServerXml11CharsetInitEP9TiXmlNode
char* CServerXml::CharsetInit(TiXmlNode* root) {
    TiXmlNode* const option = FirstChild(root, "option");
    if (!option) {
        TagError("option");
    }
    TiXmlNode* const charset = FirstChild(option, "charset");
    if (!charset) {
        TagError("charset");
    }
    const char* const type = Attribute(charset, "type");
    if (!type) {
        TagError("type");
    }
    if (std::string(type) != "kor" && std::string(type) != "chn" &&
        std::string(type) != "jpn" && std::string(type) != "usa" &&
        std::string(type) != "twn") {
        TagError("type");
    }
    charset_ = type;
    return const_cast<char*>(charset_.c_str());
}

// IDA_EA: 0x80C5FAE
// IDA_NAME: _ZN13np_server_xml10CServerXml9EventLoadEP9TiXmlNode
char CServerXml::EventLoad(TiXmlNode* root) {
    TiXmlNode* const event_strings = FirstChild(root, "event_str");
    if (!event_strings) {
        return static_cast<char>(std::puts("[CServerXml] <event_str> Tag Skip!!"));
    }
    bool loaded = false;
    for (std::vector<TiXmlNode>::iterator it = event_strings->children.begin();
         it != event_strings->children.end(); ++it) {
        if (it->name != "event") {
            continue;
        }
        loaded = true;
        const int id = AttributeInt(&*it, "id");
        RGBALoad(id, &*it);
        TiXmlNode* const localized = FirstChild(&*it, charset_.c_str());
        if (!localized) {
            std::printf("%s Tag Error\n", charset_.c_str());
            std::exit(-1);
        }
        StrPunish(id, Attribute(localized, "start_msg"), E_STRING_EVENT_START);
        StrPunish(id, Attribute(localized, "end_msg"), E_STRING_EVENT_END);
    }
    if (!loaded) {
        TagError("event");
    }
    return 1;
}

// IDA_EA: 0x80C6164
// IDA_NAME: _ZN13np_server_xml10CServerXml8RGBALoadEiP9TiXmlNode
int CServerXml::RGBALoad(int id, TiXmlNode* event_node) {
    TiXmlNode* const color = FirstChild(event_node, "color");
    if (!color) {
        std::printf("%s Tag Error\n", charset_.c_str());
        std::exit(-1);
    }
    const unsigned int red = static_cast<unsigned char>(AttributeInt(color, "red"));
    const unsigned int green = static_cast<unsigned char>(AttributeInt(color, "green"));
    const unsigned int blue = static_cast<unsigned char>(AttributeInt(color, "blue"));
    const unsigned int alpha = static_cast<unsigned char>(AttributeInt(color, "alpha"));
    event_rgba_.insert(std::make_pair(
        id, static_cast<int>(red | (green << 8) | (blue << 16) | (alpha << 24))));
    return 0;
}

// IDA_EA: 0x80C62B6
// IDA_NAME: _ZN13np_server_xml10CServerXml11ProcessLoadEP9TiXmlNode
char CServerXml::ProcessLoad(TiXmlNode* root) {
    TiXmlNode* const strings = FirstChild(root, "str");
    if (!strings) {
        return static_cast<char>(std::puts("[CServerXml] <str> Tag Skip!!"));
    }
    bool loaded = false;
    for (std::vector<TiXmlNode>::iterator it = strings->children.begin();
         it != strings->children.end(); ++it) {
        if (it->name != "string") {
            continue;
        }
        loaded = true;
        const int id = AttributeInt(&*it, "id");
        TiXmlNode* const localized = FirstChild(&*it, charset_.c_str());
        if (!localized) {
            std::printf("%s Tag Error\n", charset_.c_str());
            return 0;
        }
        StrPunish(id, Attribute(localized, "text"), E_STRING_NORMAL);
    }
    if (!loaded) {
        TagError("string");
    }
    return 1;
}

// IDA_EA: 0x80C6402
// IDA_NAME: _ZN13np_server_xml10CServerXml9StrPunishEiPKcNS_12_eStringTypeE
void CServerXml::StrPunish(int id, const char* text, _eStringType type) {
    if (!text) {
        return;
    }
    switch (type) {
        case E_STRING_NORMAL:
            strings_.insert(std::make_pair(id, std::string(text)));
            break;
        case E_STRING_EVENT_START:
            event_start_strings_.insert(std::make_pair(id, std::string(text)));
            break;
        case E_STRING_EVENT_END:
            event_end_strings_.insert(std::make_pair(id, std::string(text)));
            break;
    }
}

// IDA_EA: 0x80C6668
// IDA_NAME: _ZNK13np_server_xml10CServerXml15GetServerStringEiPb
std::string CServerXml::GetServerString(int id, bool* found) const {
    const std::map<int, std::string>::const_iterator it = strings_.find(id);
    if (it == strings_.end()) {
        if (found) {
            *found = false;
        }
        return std::string();
    }
    if (found) {
        *found = true;
    }
    return it->second;
}

// IDA_EA: 0x80C67A0
// IDA_NAME: _ZNK13np_server_xml10CServerXml12GetEventRGBAEi
int CServerXml::GetEventRGBA(int id) const {
    const std::map<int, int>::const_iterator it = event_rgba_.find(id);
    return it == event_rgba_.end() ? 0 : it->second;
}

// IDA_EA: 0x80C6822
// IDA_NAME: _ZNK13np_server_xml10CServerXml14GetEventStringEiNS_12_eStringTypeEPb
std::string CServerXml::GetEventString(int id,
                                        _eStringType type,
                                        bool* found) const {
    const std::map<int, std::string>* source = 0;
    if (type == E_STRING_EVENT_START) {
        source = &event_start_strings_;
    } else if (type == E_STRING_EVENT_END) {
        source = &event_end_strings_;
    } else {
        return std::string();
    }
    const std::map<int, std::string>::const_iterator it = source->find(id);
    if (it == source->end()) {
        if (found) {
            *found = false;
        }
        return std::string();
    }
    if (found) {
        *found = true;
    }
    return it->second;
}

}  // namespace np_server_xml

np_server_xml::CServerXml g_ServerString_;

int LoadServerStringXml() {
    return g_ServerString_.StrLoading();
}
