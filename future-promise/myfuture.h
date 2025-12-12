#pragma once

#include <thread>
#include <future> // for future error codes

#include "myshared_state.h"

template <typename res>
class mypromise;

template <typename Ret>
class myfuture{
    std::weak_ptr<myshared_state<Ret>> s_;
    friend class mypromise<Ret>;
    
    myfuture(std::shared_ptr<myshared_state<Ret>> shared) : s_(shared){};

    public:
        myfuture(const myfuture&) = delete;
        myfuture* operator=(const myfuture&) = delete;

        myfuture(myfuture&&) = default;
        myfuture& operator=(myfuture&&) = default;

        explicit myfuture() noexcept = default;

        Ret get(){

            auto ptr = s_.lock();

            if(!ptr){
                throw std::future_error(std::future_errc::no_state);
            }

            if(ptr->retr_){
                throw std::future_error(std::future_errc::future_already_retrieved);
            }
            
            std::unique_lock<std::mutex> lk{ptr->m_};

            while(!ptr->ready_){
                ptr->cond_.wait(lk);
            }

            ptr->retr_ = true;

            if(ptr->exc_){
                std::rethrow_exception(ptr->exc_);
            }

            if(ptr->val_.has_value()){
                return std::move(ptr->val_.value());
            }
            
            throw std::future_error(std::future_errc::broken_promise);
        }

        void wait(){
            
            auto ptr = s_.lock();

            if(!ptr){
                throw std::future_error(std::future_errc::no_state);
            }

            std::unique_lock<std::mutex> lk{ptr->m_};

            while(!ptr->ready_){
                ptr->cond_.wait(lk);
            }
        }
};