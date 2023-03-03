#ifndef SINGLETON_H
#define SINGLETON_H
/**
 *  @file   PublicSingleton.h
 *  @brief  A public base class for singleton object.
 *  @date   Apri 22, 2022
 *  @ref    https://github.com/BoomingTech/Pilot/blob/main/engine/source/runtime/core/base/public_singleton.h
 **/
#include <type_traits>

#define NonCopyable(classname) \
    public:\
    classname(const classname &) = delete;\
    classname& operator=(const classname &) = delete;

namespace GComponent{
    template<typename T>
    class SingletonBase
    {
    protected:
        SingletonBase() = default;
    public:
        static T& getInstance() noexcept(std::is_nothrow_constructible<T>::value){
            static T instance;
            return instance;
        }
        virtual ~SingletonBase() noexcept               = default;
        SingletonBase(const SingletonBase&)             = delete;
        SingletonBase& operator=(const SingletonBase&)  = delete;
    };
}

#endif // SINGLETON_H
