#include "future-promise/myfuture.h"
#include "future-promise/mypromise.h"

#include <thread>


template <typename Callable, typename ... Args>
inline myfuture<std::invoke_result_t<Callable, Args...>> async(Callable&& c, Args&& ... args){
    
    std::packaged_task<std::invoke_result_t<Callable, Args...>(Args...)> pt(std::forward<Callable>(c));
    auto f = pt.get_future();

    std::thread t{std::move(pt), std::forward<Args>(args)...};

    t.detach();

    return f;
}