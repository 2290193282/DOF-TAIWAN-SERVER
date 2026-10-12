#ifndef OLDSRV_GLOBAL_SERVER_XML_RUNTIME_H_
#define OLDSRV_GLOBAL_SERVER_XML_RUNTIME_H_

#include <map>
#include <string>

class TiXmlNode;

namespace np_server_xml {

// The original server string resource uses these three distinct maps.  The
// values are populated from server_str.xml at startup, never synthesized at a
// lookup boundary.
enum _eStringType {
    E_STRING_NORMAL = 0,
    E_STRING_EVENT_START = 1,
    E_STRING_EVENT_END = 2
};

class CServerXml {
public:
    // IDA_EA: 0x80C5A14
    // IDA_NAME: _ZN13np_server_xml10CServerXmlC2Ev
    CServerXml();

    // IDA_EA: 0x80C5B26
    // IDA_NAME: _ZN13np_server_xml10CServerXmlD2Ev
    ~CServerXml();

    // IDA_EA: 0x80C5C3E
    // IDA_NAME: _ZN13np_server_xml10CServerXml10InitStringEv
    int InitString();

    // IDA_EA: 0x80C5CA8
    // IDA_NAME: _ZN13np_server_xml10CServerXml10StrLoadingEv
    int StrLoading();

    // IDA_EA: 0x80C5D38
    // IDA_NAME: _ZN13np_server_xml10CServerXml10StrLoadingESs
    int StrLoading(std::string file_name);

    // IDA_EA: 0x80C5E0E
    // IDA_NAME: _ZN13np_server_xml10CServerXml11CharsetInitEP9TiXmlNode
    char* CharsetInit(TiXmlNode* root);

    // IDA_EA: 0x80C5FAE
    // IDA_NAME: _ZN13np_server_xml10CServerXml9EventLoadEP9TiXmlNode
    char EventLoad(TiXmlNode* root);

    // IDA_EA: 0x80C6164
    // IDA_NAME: _ZN13np_server_xml10CServerXml8RGBALoadEiP9TiXmlNode
    int RGBALoad(int id, TiXmlNode* event_node);

    // IDA_EA: 0x80C62B6
    // IDA_NAME: _ZN13np_server_xml10CServerXml11ProcessLoadEP9TiXmlNode
    char ProcessLoad(TiXmlNode* root);

    // IDA_EA: 0x80C6402
    // IDA_NAME: _ZN13np_server_xml10CServerXml9StrPunishEiPKcNS_12_eStringTypeE
    void StrPunish(int id, const char* text, _eStringType type);

    // IDA_EA: 0x80C6668
    // IDA_NAME: _ZNK13np_server_xml10CServerXml15GetServerStringEiPb
    std::string GetServerString(int id, bool* found) const;

    // IDA_EA: 0x80C67A0
    // IDA_NAME: _ZNK13np_server_xml10CServerXml12GetEventRGBAEi
    int GetEventRGBA(int id) const;

    // IDA_EA: 0x80C6822
    // IDA_NAME: _ZNK13np_server_xml10CServerXml14GetEventStringEiNS_12_eStringTypeEPb
    std::string GetEventString(int id,
                               _eStringType type,
                               bool* found) const;

private:
    std::string charset_;
    std::string source_file_;
    std::map<int, std::string> strings_;
    std::map<int, std::string> event_start_strings_;
    std::map<int, std::string> event_end_strings_;
    std::map<int, int> event_rgba_;
};

}  // namespace np_server_xml

// IDA data: g_ServerString_ (constructed before Init and loaded by Init).
extern np_server_xml::CServerXml g_ServerString_;

// The restored app bootstrap invokes this exact resource load after global
// data has been initialized, matching the old server's startup ordering.
int LoadServerStringXml();

#endif  // OLDSRV_GLOBAL_SERVER_XML_RUNTIME_H_
