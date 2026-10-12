#include "infra/security/crypto/crypto_graph.h"


// IDA_EA: 0x809DEF0
// IDA_NAME: _ZN12ICryptoGraphC2E17CRYPTO_GRAPH_TYPE
// DEMANGLED: ICryptoGraph::ICryptoGraph(CRYPTO_GRAPH_TYPE)
ICryptoGraph::ICryptoGraph(CRYPTO_GRAPH_TYPE graph_type)
    : key_(), graph_type_(graph_type), mode_(CRYPTO_MODE_ECB), context_(0) {}

// IDA_EA: 0x809DF4C
// IDA_NAME: _ZN12ICryptoGraphC2ERKS_
// DEMANGLED: ICryptoGraph::ICryptoGraph(ICryptoGraph const&)
ICryptoGraph::ICryptoGraph(const ICryptoGraph& rhs)
    : key_(), graph_type_(rhs.graph_type_), mode_(CRYPTO_MODE_ECB),
      context_(0) {}

// IDA_EA: 0x809DF08
// IDA_NAME: _ZN12ICryptoGraphD2Ev
// DEMANGLED: ICryptoGraph::~ICryptoGraph()
// IDA_EA: 0x809DF2E
// IDA_NAME: _ZN12ICryptoGraphD0Ev
// DEMANGLED: ICryptoGraph::~ICryptoGraph() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
ICryptoGraph::~ICryptoGraph() {}

// IDA_EA: 0x809DF66
// IDA_NAME: _ZN12ICryptoGraphaSERKS_
// DEMANGLED: ICryptoGraph::operator=(ICryptoGraph const&)
ICryptoGraph& ICryptoGraph::operator=(const ICryptoGraph& rhs) {
    if (this != &rhs) {
        graph_type_ = rhs.graph_type_;
    }
    return *this;
}

// IDA_EA: 0x809DF82
// IDA_NAME: _ZNK12ICryptoGraph10GetApiTypeEv
// DEMANGLED: ICryptoGraph::GetApiType(void) const
CRYPTO_GRAPH_TYPE ICryptoGraph::GetApiType() const {
    return graph_type_;
}
