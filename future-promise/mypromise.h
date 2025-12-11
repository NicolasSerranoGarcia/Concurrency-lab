#pragma once

#include <thread>
#include <memory>
#include <future> //for future_error codes

#include "myshared_state.h"
#include "myfuture.h"


template <typename res>
class mypromise{
    std::shared_ptr<myshared_state<res>> s_;

    public:
        explicit mypromise() noexcept : s_{std::make_shared<myshared_state<res>>()} {};
        ~mypromise(){
            if(!s_->ready_){
                s_->cond_.notify_all();
            }
        }

        myfuture<res> get_future(){
            std::lock_guard<std::mutex> lk{s_->m_};
            if(s_->future_taken_){
                throw std::future_error(std::future_errc::future_already_retrieved);
            }

            s_->future_taken_ = true;
            return myfuture<res>(s_);
        }

        void set_value(res val){
            std::lock_guard<std::mutex> lk{s_->m_};
            if(s_->val_.has_value() || s_->exc_){
                throw std::future_error(std::future_errc::promise_already_satisfied);
            }
            s_->val_ = val;
            s_->ready_ = true;
            s_->cond_.notify_all();
        }

        template<typename exc>
        void set_exception(exc exception){
            std::lock_guard<std::mutex> lk{s_->m_};
            if(s_->exc_ || s_->val_.has_value()){
                throw std::future_error(std::future_errc::promise_already_satisfied);
            }
            s_->ready_ = true;
            s_->exc_ = std::make_exception_ptr(exception); 
            s_->cond_.notify_all();
        }
};