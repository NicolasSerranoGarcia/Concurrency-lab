#include <thread>
#include <iostream>
#include <mutex>

struct invalidHierarchyException : std::logic_error {
    invalidHierarchyException(std::string msg) : std::logic_error{msg} {};

    std::string what(){
        return what();
    }
};

//the own mutex has an assigned level of hierarchy, which must be met by the caller thread
class hierarchicalMutex{
    std::mutex _internalMutex;

    //supposing it is unique amongst mutexes
    unsigned long int _mutexHierarchyValue;
    unsigned long int _previousThreadHierarchyValue;
    static thread_local unsigned long _callerThreadHierarchyValue;

    void checkForHierarchyValue(){
        //can't lock it
        if(_callerThreadHierarchyValue <= _mutexHierarchyValue){
            throw invalidHierarchyException("Mutex hierarchy violated!");
        }
    }

    void updateThreadHierarchyValue(){
        _previousThreadHierarchyValue = _callerThreadHierarchyValue;
        //adopts the value of the mutex
        _callerThreadHierarchyValue = _mutexHierarchyValue;
    }

    public:
        explicit hierarchicalMutex() noexcept : _mutexHierarchyValue{ULONG_MAX}, _previousThreadHierarchyValue{0} {};

        explicit hierarchicalMutex(unsigned long int index) noexcept : _mutexHierarchyValue{index}, _previousThreadHierarchyValue{0} {};

        void lock(){
            checkForHierarchyValue();
            _internalMutex.lock();
            updateThreadHierarchyValue();
        }

        void unlock(){
            if(_callerThreadHierarchyValue != _mutexHierarchyValue){
                throw invalidHierarchyException("Released mutex in incorrect order!");
            }

            _callerThreadHierarchyValue = _previousThreadHierarchyValue;
            _internalMutex.unlock();
        }

        bool tryLock(){
            checkForHierarchyValue();
            //try lock atomizes the operation of locking. It checks for lockness and if it isn't also locks the mutex. 
            //instead of giving two functions (is_locked and lock), this atomizes it into one. This is to avoid data
            //races in client code
            if(!_internalMutex.try_lock()){
                return false;
            }

            updateThreadHierarchyValue();
            return true;
        }
};

thread_local unsigned long hierarchicalMutex::_callerThreadHierarchyValue = ULONG_MAX;