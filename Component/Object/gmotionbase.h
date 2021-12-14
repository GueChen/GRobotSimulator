#ifndef GMOTIONBASE_H
#define GMOTIONBASE_H

namespace GComponent {

enum class InterpolationEnum{
    Simple,
    Quadratic,
    Cubic,
    Quintic,
    Trapezoid
};

class GMotionBase
{
public:
    GMotionBase();
};

}
#endif // GMOTIONBASE_H
