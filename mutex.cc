#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <algorithm>

std::vector<int> vec;
std::mutex mut;

void add(int num){

    //this operates directly on the mutex, changing it's state when constructing (locks mut). When it goes out of scope, it unlocks it,
    //leaving other threads that tried locking the mutex now suitable to take the lock.
    std::lock_guard<std::mutex> guard(mut);
    vec.push_back(num);
}

bool contains(int num){

    //problem. When visiting all of the entrances of the vector, you are visiting each memory address where the data is located.
    //If you try multithreading this and adding a new element, you are generating a data race, because there is a possibility you are
    //accessing the same memory addres in two threads, without before-after order, non-atomically and without syncronization = data race.
    //For this, guard the access to executing the next line
    std::lock_guard<std::mutex> guard(mut);
    //note that the use of lock_guard is strictly for ensuring the mut gets unlocked by the thread doing the operation when it no longer needs 
    //to operate on the critical zone (if we just used mut.lock() std::find for example could throw without us knowing, which would exit the function
    //early, leaving the mutex locked. This would permanently disable the access to the critical zone the mutex was guarding to all the threads)
    return vec.end() == std::find(vec.begin(), vec.end(), num);
}

int main(){

}