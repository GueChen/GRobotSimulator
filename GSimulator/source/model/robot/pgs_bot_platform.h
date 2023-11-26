#ifndef __PGS_BOT_PLATFORM_H
#define __PGS_BOT_PLATFORM_H

#include <memory>
#include "model/model.h"

namespace GComponent {
    class PGS_BOT_PLATFORM : public Model
    {
    public:
        explicit PGS_BOT_PLATFORM(Mat4 transform = Mat4::Identity());
        ~PGS_BOT_PLATFORM() = default;


    private:
        void InitializeModelResource();
        void InitializeModel();

    private:
        Model* aubo_i3_robot_r_;
        Model* aubo_i3_robot_l_;
    };

}

#endif