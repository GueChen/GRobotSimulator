#ifndef YDLPlatform_H
#define YDLPlatform_H

#include <memory>
#include "model/model.h"

namespace GComponent {
    class YDLPlatform : public Model
    {
    public:
        explicit YDLPlatform(Mat4 transform = Mat4::Identity());
        ~YDLPlatform() = default;


    private:
        void InitializeModelResource();
        void InitializeModel();

    private:
        Model* aubo_i5_robot_;
    };

}


#endif // YDLPlatform_H