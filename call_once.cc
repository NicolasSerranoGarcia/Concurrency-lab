//// first approach for lazy init: mutex
//// Ensures thread safe, but generates overhead not only for the initialization, but also for the access once it is initialized.
//// The program behaves linearly (non-parallel) because the mutex blocks the access to the data.

// #include <thread>
// #include <mutex>

// struct big_object{};

// std::unique_ptr<big_object> singleton;
// std::mutex guard_for_singleton;

// //should only be called once
// void init_big_object(){
//     singleton = std::make_unique<big_object>();
// };

// void operate_on_big_object(){
//     std::lock_guard<std::mutex> lk{guard_for_singleton};
//     if(!singleton){
//         init_big_object();
//     }
//     //do_stuff()
// };


// //second approach for lazy init: double check. It reduces overhead to only the initialization, making consequent accesses much faster (only an if)
// //but on the initialization it generates a data race. The initialization is guarded, but the check on the first if is not. This generates a data race
// //condition: There is a situation where there might be a simultaneous access by different threads to a memory address, where at least one of them
// //is in write mode (the initialization) and there is no guard or happens-before guard (mutex)

// #include <thread>
// #include <mutex>

// struct big_object{};

// std::unique_ptr<big_object> singleton;
// std::mutex guard_for_singleton;

// //should only be called once
// void init_big_object(){
//     singleton = std::make_unique<big_object>();
// };

// void operate_on_big_object(){
//     if(!singleton){
//         std::lock_guard<std::mutex> lk{guard_for_singleton};
//         if(!singleton){
//             init_big_object();
//         }
//     }
//     //do_stuff()
// };


// // third approach for lazy init: call_once and once_flag. Elegant asf. It is also the best from both worlds. There is no data race in the initialization
// // neither in the runtime access, so it is thread safe. It is also as fast as double check, because once the call has been made, the flag indicates
// // so, making the internal check only an if for the flag. This is thanks to the CAS mechanism. Initialization is done only by 1 thread. This thread
// // also changes the flag to be in INITIALIZING mode, which makes the other threads trying to init be in wait or spin mode. Once the call has finished
// // the threads are woken up. All of this happens ATOMICALLY, so if thread A is not the initializer, the only thing that it sees is the flag go from 
// // UNINITIALIZED to INITIALIZING (where it gets blocked) and then woken up to initialized. Overall, there is no data race between multiple caller threads.
// // and the data is available to all of them when they get resumed. Consequent accesses will compare the flag to be initialized and will just return, 
// //  as the flag states that the object is already initialized. 

#include <thread>
#include <mutex>
#include <shared_mutex>

struct big_object{};

std::unique_ptr<big_object> singleton;
std::once_flag flag_for_singleton;

//should only be called once
void init_big_object(){
    singleton = std::make_unique<big_object>();
};

void operate_on_big_object(){
    std::call_once(flag_for_singleton, init_big_object); //no args for init_big_object
    //do_stuff()
    std::shared_mutex s;
    std::mutex r;
};

int main(){
    std::thread t1(operate_on_big_object);
    std::thread t2(operate_on_big_object);

    t1.join();
    t2.join();
}