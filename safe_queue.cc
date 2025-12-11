#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <memory>
#include <iostream>
#include <future>

template <typename T>
class threadsafeQueue{
    private:
        std::queue<T> queue_;
        mutable std::mutex m_;
        std::condition_variable var;
    public:
        explicit threadsafeQueue() noexcept {}

        bool empty() const{
            std::lock_guard<std::mutex> lk(m_);
            return queue_.empty();
        }

        //adding a scope to the lock guard I think will give priority to the try_pop operations if there happens to be one waiting on the mutex to
        //be unblocked. When the mutex gets destroyed because of the scope, if there was a try_pop waiting, the only thing it has to do
        //is acquire the lock that is now free. As there is also a newly pushed element, the function does manage to read front() and pop(), leaving
        //the queue empty again. If there was also a wait_and_pop() thread waiting to be waken up, the action of notifying, waking the thread up
        //and then trying to acquire the lock will probably be slower than just acquiring the lock (try_pop). This makes wait_and_pop also wait
        //for the try_pop operation to finish, which will leave the queue empty again. When the wait_and_pop finally acquires the lock, it will check
        //the condition (not empty), but as the try_lock already emptied the queue, the condition will not be satisfied, unlocking the thread and putting
        //it to sleep again. So in the case there are two threads, one try_pop and other wait_and_pop, following this line of reasoning, try_pop willp have
        //priority
        //This might be true in practice. The C++ standard does not guarantee any order in the scheduling of the threads, so when there are multiple
        //threads waiting to acquire a lock on a mutex, there is no guarantee in which will acquire it.
        void push(T value){
            {
                std::lock_guard<std::mutex> lk(m_);
                queue_.push(value);
            }
            var.notify_one();
            std::cout << "pushed" << std::endl;
        }

        bool try_pop(T& value){
            std::lock_guard<std::mutex> lk(m_);
            if(queue_.empty()){
                return false;
            }
            value = queue_.front();
            queue_.pop();
            return true;
        }
        
        std::shared_ptr<T> try_pop(){
            std::lock_guard<std::mutex> lk(m_);
            if(queue_.empty()){
                return nullptr;
            }
            std::shared_ptr<T> ptr = std::make_shared<T>(queue_.front());
            queue_.pop();
            std::cout << "popped with try" << std::endl;
            return ptr;
            std::promise<int> s;
        }

        void wait_and_pop(T& value){
            std::unique_lock<std::mutex> lk(m_);
            var.wait(lk, [this](){ return !queue_.empty(); });
            value = queue_.front();
            queue_.pop();
        }
        
        std::shared_ptr<T> wait_and_pop(){
            std::unique_lock<std::mutex> lk(m_);
            var.wait(lk, [this](){ return !queue_.empty(); });
            std::shared_ptr<T> ptr = std::make_shared<T>(queue_.front());
            queue_.pop();
            std::cout << "popped with wait" << std::endl;
            return ptr;
        }

};

threadsafeQueue<int> q;

int main(){


    while(true){
        std::thread t1{[](){ q.push(5); }};
        std::thread t3{[](){ q.try_pop(); }};
        std::thread t2{[](){ q.wait_and_pop(); }};

        t1.join();
        t3.join();
        t2.join();
    }

}