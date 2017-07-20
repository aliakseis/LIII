#pragma once

#include <utility>
#include <atomic>

template <typename T, typename = void>
struct has_init_function : std::false_type {};
template <typename T>
struct has_init_function<T, decltype(std::declval<T>().init())>
    : std::true_type {};

template <typename T>
class Singleton
{
public:
    static T& instance()
    {
        return do_instance(has_init_function<T>());
    }

private:
    static T& do_instance(std::false_type)
    {
        static T instance_;
        return instance_;
    }
    static T& do_instance(std::true_type)
    {
        static T instance_;
        static std::atomic_bool once;
        if (!once.exchange(true))
            instance_.init();
        return instance_;
    }
protected:
    Singleton() {}
    ~Singleton() {}
public:
    Singleton(Singleton const &) = delete;
    Singleton& operator=(Singleton const &) = delete;
};