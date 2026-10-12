#ifndef OLDSRV_GLOBAL_STATISTICS_NAT_TYPE_H_
#define OLDSRV_GLOBAL_STATISTICS_NAT_TYPE_H_

#include <stdint.h>

#include <map>

// IDA unit: 99_misc_global/StatisticsNatType.  GameWorld constructs this
// exact map object at +0x11C and constructs the next user map at +0x134.
class StatisticsNatType {
public:
    struct NAT_TYPE_PEER_TO_PEER {
        // IDA_EA: 0x8610262
        // IDA_NAME: _ZN17StatisticsNatType21NAT_TYPE_PEER_TO_PEERC2Eii
        // DEMANGLED: StatisticsNatType::NAT_TYPE_PEER_TO_PEER::NAT_TYPE_PEER_TO_PEER(int,int)
        NAT_TYPE_PEER_TO_PEER(int nat_type, int nat_type_other);

        // IDA_EA: 0x861021E
        // IDA_NAME: _ZNK17StatisticsNatType21NAT_TYPE_PEER_TO_PEERltERKS0_
        // DEMANGLED: StatisticsNatType::NAT_TYPE_PEER_TO_PEER::operator<(StatisticsNatType::NAT_TYPE_PEER_TO_PEER const&)const
        bool operator<(const NAT_TYPE_PEER_TO_PEER& other) const;

        int nat_type;
        int nat_type_other;
    };

    struct RESULT_COUNT {
        // IDA_EA: 0x8610294
        // IDA_NAME: _ZN17StatisticsNatType12RESULT_COUNTC2Ev
        // DEMANGLED: StatisticsNatType::RESULT_COUNT::RESULT_COUNT(void)
        RESULT_COUNT();

        int success_count;  // +0x00
        int fail_count;     // +0x04
    };

    // IDA_EA: 0x860C704
    // IDA_NAME: _ZN17StatisticsNatTypeC2Ev
    // DEMANGLED: StatisticsNatType::StatisticsNatType(void)
    StatisticsNatType();

    // IDA_EA: 0x860C718
    // IDA_NAME: _ZN17StatisticsNatTypeD2Ev
    // DEMANGLED: StatisticsNatType::~StatisticsNatType()
    ~StatisticsNatType();

    // IDA_EA: 0x860C7DA
    // IDA_NAME: _ZN17StatisticsNatType5ResetEv
    // DEMANGLED: StatisticsNatType::Reset(void)
    int Reset();

    // IDA_EA: 0x860C86C
    // IDA_NAME: _ZN17StatisticsNatType15IncreaseNatTypeEiih
    // DEMANGLED: StatisticsNatType::IncreaseNatType(int,int,unsigned char)
    RESULT_COUNT* IncreaseNatType(int nat_type,
                                  int nat_type_other,
                                  unsigned char result);

    // IDA_EA: 0x860C968
    // IDA_NAME: _ZN17StatisticsNatType13UpdateNatTypeEv
    // DEMANGLED: StatisticsNatType::UpdateNatType(void)
    int UpdateNatType();

private:
    typedef std::map<NAT_TYPE_PEER_TO_PEER, RESULT_COUNT*> NatTypeMap;

    NatTypeMap nat_type_counts_;
};

static_assert(sizeof(StatisticsNatType::NAT_TYPE_PEER_TO_PEER) == 0x08u,
              "NAT_TYPE_PEER_TO_PEER must retain its two-int ABI");
static_assert(sizeof(StatisticsNatType::RESULT_COUNT) == 0x08u,
              "RESULT_COUNT must retain its two-counter ABI");
#if defined(__i386__)
static_assert(sizeof(StatisticsNatType) == 0x18u,
              "StatisticsNatType must retain the old i386 std::map ABI");
#endif

#endif  // OLDSRV_GLOBAL_STATISTICS_NAT_TYPE_H_
