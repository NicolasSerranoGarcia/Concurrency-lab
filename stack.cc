#include <deque>
#include <mutex>
#include <thread>
#include <memory>
#include <iostream>

//on a classic Queue
// front → [ e0 ][ e1 ][ e2 ] ... [ en ] ← back
//            (out)             (in)
//
//On a classic Deque, you can do both operations on both sides.
// But still, push_front and pop_front mean operating where the values would normal get out
// and push_back and pop_back mean operating where the values would normally get IN

//just use a deque
template<typename Type>
class stack{
    std::deque<Type> _cont;

    public:
        explicit stack() noexcept{};

        void push(const Type& elem){
            _cont.push_back(elem);
        }

        void push(Type&& elem){
            _cont.push_back(std::move(elem));
        }

        //only eliminates the last element. It is convenient to do it this way for concurrency purposes
        void pop(){
            _cont.pop_back();
        }

        const Type& top(){
            return _cont.back();
        }

        bool empty(){
            return _cont.empty();
        }
};


//in the case of the classical stack, operations get completed sequentially (for example, if you tried to access the top element
// and then poping from the stack, there would be no problem, as those operations get executed in order and in a single thread)
//the problem with multithreaded stack comes from the fact that while an operation is being executed by a thread (breaking invariance)
//another thread can start an operation that vulnerates that invariant (for example,
// if(!stack.empty()){
//  const Type elem = stack.top();
//  stack.pop();
//}
//in the example, it might happen that a thread checks for emptyness (at that moment there is one element, so it is not empty)
//then, as the stack is not empty, the thread enters the if, accessing the top element. 
// But if a thread pops the only element in the middle of the operation between empty and top(), then the thread will be in an invalid state.
//This is easily solved with mutexes.
template<typename Type>
class parallelStack{
    stack<Type> _stack;
    std::mutex _stackMutex;

    public:
        explicit parallelStack() noexcept{};

        void push(const Type& elem){
            std::lock_guard<std::mutex> guard(_stackMutex);
            _stack.push(elem);
        }

        void push(Type&& elem){
            std::lock_guard<std::mutex> guard(_stackMutex);
            _stack.push(std::move(elem));
        }

        std::shared_ptr<Type> pop(){
            std::lock_guard<std::mutex> guard(_stackMutex);

            if(_stack.empty()){
                //just indicate
                throw std::exception{};
            }
            //if this throws we are safe of keeping the _stack in a valid state, because we are just accessing to the top element, but 
            //not poping it. The bad thing is that the user won't be able to get the value, but at least the integrity is preserved 
            const std::shared_ptr<Type> ptr = std::make_shared<Type>(_stack.top());
            _stack.pop();
            return ptr;
        }

        void pop(Type& ref){
            std::lock_guard<std::mutex> guard(_stackMutex);

            if(_stack.empty()){
                //just indicate
                throw std::exception{};
            }
            //if this throws we are safe of keeping the _stack in a valid state, because we are just accessing to the top element, but 
            //not poping it. The bad thing is that the user won't be able to get the value, but at least the integrity is preserved 
            ref = _stack.top();
            _stack.pop();
        }

        bool empty(){
            std::lock_guard<std::mutex> guard(_stackMutex);
            return _stack.empty();
        }

        void swap(parallelStack<Type>& A, parallelStack<Type>& B) const{

            if(&A == &B){
                return;
            }


            //we ensure we got both mutexes locked without deadlocks
            std::lock(A._stackMutex, B._stackMutex);
            //then we wrap the mutexes unlocking in an object. 
            std::lock_guard<std::mutex> a(A._stackMutex, std::adopt_lock);
            std::lock_guard<std::mutex> b(B._stackMutex, std::adopt_lock);

            //The lock, then lock_guard can be replaced with simply:
            // std::scoped_lock<std::mutex, std::mutex> s(A._stackMutex, B._stackMutex)
            //or even std::scoped_lock(A._stackMutex, B._stackMutex), with type deduction
        }
};

parallelStack<int> st;

void write(){
    st.push(3);
    std::cout << "pushed 3 to the st" << std::endl;
}

std::shared_ptr<int> get(){
    auto ptr = st.pop();
    std::cout << "popped from the st " << std::to_string(*ptr.get()) << std::endl; 
    return ptr;
}

int main(){

    //this is a PERFECT example for the use of joining threads. If the function that gets the value cannot do it because it is empty, 
    //it returns early, BUT we are not joining the thread
    std::thread t1(write);
    std::thread t2;
    try{
        std::thread t2 = std::thread{get};
    } catch(...){
        return 0;
    }

    t1.join();
    t2.join();
}