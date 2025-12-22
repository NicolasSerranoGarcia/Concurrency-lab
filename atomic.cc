#include <atomic>
#include <thread>
#include <iostream>

class spinlock_mutex{
    private:
        std::atomic_flag f;
    public:
        spinlock_mutex() : f(ATOMIC_FLAG_INIT) {};

        void lock(){
            //returns the previous value and sets the flag to true. So initially, the first lock will not enter the loop, but it will set the flag,
            //all atomically, so the next thread will see the flag to set
            while(f.test_and_set(std::memory_order_acquire)){};
        }

        void unlock(){
            f.clear(std::memory_order_release);
        }
};

int x = 0;

void foo(){
    spinlock_mutex mut;

    mut.lock();
    if(x == 0){
        x = 2;
    } else if(x == 2){
        std::cout << "id: " << std::this_thread::get_id() << std::endl;
    }
    mut.unlock();
}


int main(){
    std::thread t1(foo);
    std::cout << t1.get_id() << std::endl;
    std::thread t2(foo);
    std::cout << t2.get_id() << std::endl;

    t1.join();
    t2.join();

    std::atomic<bool> atomic;
    bool expected = false;
    bool desired = true;

    //the parameter expected is IN and OUT.
    //If expected == atomic.val
    // atomic = desired;    <- IMPORTANT 
    // return true; 
    //else
    // expected = atomic.val;
    // return false;
    atomic.compare_exchange_weak(expected, desired);

    //the use of the instruction is to change the value of a variable if you have an expected value.

    //it is normally used inside a loop

    while(!atomic.compare_exchange_weak(expected, desired) && !expected){ //it doesn't matter the value of the atomic, because we are searching for a specific state of it. 
        //this will happen while atomic != expected 
        //it also happens that, on each iteration, expected is updated with the value of atomic.
        //this gives us freedom to perform any further calculations based on the value, in case that other threads are changing it. 
    }

    // at this point atomic == desired

    //in any case, it is better off to think it in a higher level, in practical terms.

    //you have an atomic variable. You want to change the value it stores only if the current value is equal to one you expect (expected parameter).
    //if the value you expect in the atomic coincides, then the atomic takes the value of desired. 
    //Otherwise it sets expected to be the actual value of the atomic. 
    //The output you are looking for is the fact that the variable was changed or not.

    //the difference between strong and weak is that, with weak, the function can fail even if expected == atomic. This is why it is usually interesting
    //to keep weak in a loop, and additionally check the expected value written.
}