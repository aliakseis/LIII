#pragma once

#include <utility>
#include <atomic>


template <typename T>
class Singleton
{
public:
    static T& instance()
    {
        static T instance_;
        return instance_;
    }

protected:
    Singleton() {}
    ~Singleton() {}
public:
    Singleton(Singleton const &) = delete;
    Singleton& operator=(Singleton const &) = delete;
};