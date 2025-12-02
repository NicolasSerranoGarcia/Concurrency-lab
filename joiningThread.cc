#include <thread>
#include <iostream>

struct functor{
    int& in;

    functor(int& i_) : in{i_}{};

    void operator()(){

        for(int i = in; i < 100; i++){
            std::cout << i << " " << in <<  std::endl;
        }
    }
};

class joining_thread{
    std::thread thr;
    
    public:
        joining_thread() noexcept = default;

        template<typename Callable, typename ... Args>
        joining_thread(Callable&& callable, Args&& ... args) noexcept : 
        thr{std::forward<Callable>(callable), std::forward<Args>(args)...}{};

        //move

        joining_thread(joining_thread&& other) noexcept : thr{std::move(other.thr)}{};

        
        joining_thread& operator=(joining_thread&& other) noexcept{ 
            if(joinable()){
                join();
            } 

            thr = std::move(other.thr);

            return *this;
        };



        explicit joining_thread(std::thread t) noexcept : thr{std::move(t)}{};

        joining_thread& operator=(std::thread other) noexcept{
            if(joinable()){
                join();
            }

            thr = std::move(other);

            return *this;
        }

        bool joinable() const noexcept{ return thr.joinable(); };

        void join() { thr.join(); };

        //most important. As the class owns the thread, it can manage when to join it. Even if the thread does finish earlier than when the
        //joining_thread gets out of scope, it doesn't matter. It wont call std::terminate unless the variable std::thread that is managing the 
        //thread goes out of scope being joinable, which can't happen without this joining_thread wrapper going out of scope. So this means that
        //the std::thread will NEVER call std::terminate
        ~joining_thread(){
            //the managed thread is only managed by 1 instance of this class, but it might happen that it receives a thread that is already joined
            //We do not care about if it is joinable or not when constructing joining_thread, but rather that the only object that can manage it 
            //is this one
            if(thr.joinable()){
                thr.join();
            }
        }

};

int main(){

    std::thread t{[&](){ std::cout << "Hello, World!" << std::endl; }};
    joining_thread thr(std::move(t));

    //at this point t is a valid object, ut it does not own any thread.

    //doing t.join() is UB
}