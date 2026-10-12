#include "old_game/world/region/collision_detection_runtime.h"

namespace collision_detection {

// IDA_EA: 0x85CB444
// IDA_NAME: _ZN19collision_detection5PointC2Eii
// DEMANGLED: collision_detection::Point::Point(int,int)
Point::Point(int x_value, int y_value)
    : x(x_value),
      y(y_value) {}

// IDA_EA: 0x85CB45A
// IDA_NAME: _ZN19collision_detection9RectangleC2Ev
// DEMANGLED: collision_detection::Rectangle::Rectangle(void)
Rectangle::Rectangle()
    : Point(0, 0),
      width(0),
      height(0) {}

// IDA_EA: 0x85CB492
// IDA_NAME: _ZNK19collision_detection9Rectangle7includeERKS0_
// DEMANGLED: collision_detection::Rectangle::include(collision_detection::Rectangle const&)const
bool Rectangle::include(const Rectangle& rhs) const {
    const bool left_top =
        rhs.x >= x && rhs.x <= x + width && rhs.y >= y && rhs.y <= y + height;
    if (left_top) {
        return true;
    }

    const bool right_top = rhs.x + rhs.width >= x &&
                           rhs.x + rhs.width <= x + width &&
                           rhs.y >= y && rhs.y <= y + height;
    if (right_top) {
        return true;
    }

    const bool left_bottom = rhs.x >= x && rhs.x <= x + width &&
                             rhs.y + rhs.height >= y &&
                             rhs.y + rhs.height <= y + height;
    if (left_bottom) {
        return true;
    }

    return rhs.x + rhs.width >= x && rhs.x + rhs.width <= x + width &&
           rhs.y + rhs.height >= y && rhs.y + rhs.height <= y + height;
}

}  // namespace collision_detection
