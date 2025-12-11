#pragma once

#include <mutex>
#include <shared_mutex>
#include <optional>
#include <condition_variable>

template <typename res>
class mypromise;

template <typename res>
class myfuture;


template <typename val>
class myshared_state{
    mutable std::mutex m_;
    std::condition_variable cond_;
    std::optional<val> val_;
    std::exception_ptr exc_;
    bool retr_;
    bool future_taken_;
    bool ready_;

    friend class mypromise<val>;
    friend class myfuture<val>;

    public:

        explicit myshared_state() noexcept : retr_(false), 
                                    ready_(false), future_taken_(false), val_{}, exc_{}, m_{}{};
};