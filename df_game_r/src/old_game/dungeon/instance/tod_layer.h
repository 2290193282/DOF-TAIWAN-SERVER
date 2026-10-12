#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TOD_LAYER_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TOD_LAYER_H_

#include <stdint.h>

// Canonical two-byte Tower of Despair layer value used by DB payloads and
// runtime state.  The old constructor clamps only values above 100.
struct TOD_Layer {
    // The old payload structs default-construct this value without emitting a
    // constructor call.  Keep that path trivial; the only exported scalar
    // constructor is the unsigned-short overload below.
    TOD_Layer() = default;

    // IDA_EA: 0x85FE7B4
    // IDA_NAME: _ZN9TOD_LayerC2Et
    // DEMANGLED: TOD_Layer::TOD_Layer(unsigned short)
    explicit TOD_Layer(uint16_t layer);

    // IDA_EA: 0x85FF0FA
    // IDA_NAME: _ZN9TOD_LayerC2ERKS_
    // DEMANGLED: TOD_Layer::TOD_Layer(TOD_Layer const&)
    TOD_Layer(const TOD_Layer& other);

    // IDA_EA: 0x822EC40
    // IDA_NAME: _ZNK9TOD_Layer8GetLayerEv
    // DEMANGLED: TOD_Layer::GetLayer(void)const
    uint16_t GetLayer() const;

    // IDA_EA: 0x85FE824
    // IDA_NAME: _ZN9TOD_Layer13increaseLayerEv
    // DEMANGLED: TOD_Layer::increaseLayer(void)
    int increaseLayer();

    // IDA_EA: 0x85FE852
    // IDA_NAME: _ZN9TOD_Layer10resetLayerEv
    // DEMANGLED: TOD_Layer::resetLayer(void)
    TOD_Layer* resetLayer();

    // IDA_EA: 0x85FF114
    // IDA_NAME: _ZNK9TOD_LayerltERKS_
    // DEMANGLED: TOD_Layer::operator<(TOD_Layer const&)const
    bool operator<(const TOD_Layer& other) const;

    // IDA_EA: 0x86439C8
    // IDA_NAME: _ZNK9TOD_LayerplEi
    // DEMANGLED: TOD_Layer::operator+(int)const
    TOD_Layer operator+(int value) const;

    uint16_t layer;
};

static_assert(sizeof(TOD_Layer) == 0x02u,
              "TOD_Layer IDA ABI size must stay 2");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TOD_LAYER_H_
