#include "old_game/dungeon/instance/tod_layer.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x85FE7B4
// IDA_NAME: _ZN9TOD_LayerC2Et
// DEMANGLED: TOD_Layer::TOD_Layer(unsigned short)
TOD_Layer::TOD_Layer(uint16_t layer_value) : layer(layer_value) {
    if (layer > 100u) {
        cMyTrace trace("TOD_Layer::TOD_Layer(short unsigned int)", 16, 5);
        trace("@TOD: invalid layer setting : %d", layer_value);
        layer = 1;
    }
}

// IDA_EA: 0x822EC40
// IDA_NAME: _ZNK9TOD_Layer8GetLayerEv
// DEMANGLED: TOD_Layer::GetLayer(void)const
uint16_t TOD_Layer::GetLayer() const {
    return layer;
}

// IDA_EA: 0x85FE824
// IDA_NAME: _ZN9TOD_Layer13increaseLayerEv
// DEMANGLED: TOD_Layer::increaseLayer(void)
int TOD_Layer::increaseLayer() {
    const uint16_t result = ++layer;
    if (result > 100u) {
        resetLayer();
        return layer;
    }
    return result;
}

// IDA_EA: 0x85FE852
// IDA_NAME: _ZN9TOD_Layer10resetLayerEv
// DEMANGLED: TOD_Layer::resetLayer(void)
TOD_Layer* TOD_Layer::resetLayer() {
    layer = 1;
    return this;
}

// IDA_EA: 0x85FF0FA
// IDA_NAME: _ZN9TOD_LayerC2ERKS_
// DEMANGLED: TOD_Layer::TOD_Layer(TOD_Layer const&)
TOD_Layer::TOD_Layer(const TOD_Layer& other) : layer(other.GetLayer()) {}

// IDA_EA: 0x85FF114
// IDA_NAME: _ZNK9TOD_LayerltERKS_
// DEMANGLED: TOD_Layer::operator<(TOD_Layer const&)const
bool TOD_Layer::operator<(const TOD_Layer& other) const {
    return layer < other.layer;
}

// IDA_EA: 0x86439C8
// IDA_NAME: _ZNK9TOD_LayerplEi
// DEMANGLED: TOD_Layer::operator+(int)const
TOD_Layer TOD_Layer::operator+(int value) const {
    TOD_Layer result(static_cast<uint16_t>(layer + value));
    if (result.GetLayer() > 100u) {
        result.resetLayer();
    }
    return result;
}
