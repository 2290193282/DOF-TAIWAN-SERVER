#ifndef OLDSRV_OLD_GAME_WORLD_COLLISION_DETECTION_RUNTIME_H_
#define OLDSRV_OLD_GAME_WORLD_COLLISION_DETECTION_RUNTIME_H_

namespace collision_detection {

// IDA_EA: 0x85CB444
// IDA_NAME: _ZN19collision_detection5PointC2Eii
struct Point {
    Point(int x_value, int y_value);

    int x;
    int y;
};

// IDA_EA: 0x85CB45A / 0x85CB492
// IDA_NAME: _ZN19collision_detection9RectangleC2Ev / _ZNK19collision_detection9Rectangle7includeERKS0_
struct Rectangle : public Point {
    Rectangle();
    bool include(const Rectangle& rhs) const;

    int width;
    int height;
};

}  // namespace collision_detection

#endif  // OLDSRV_OLD_GAME_WORLD_COLLISION_DETECTION_RUNTIME_H_
