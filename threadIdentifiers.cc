#include <thread>
#include <iostream>

void smth(){
    std::cout << "yo" << std::endl;
}


int main(){
    
    std::cout << std::this_thread::get_id() << std::endl;
    
    std::thread t(smth);

    std::cout << t.get_id() << std::endl;

    t.join();

    auto res = t.get_id();
    std::cout << res << std::endl;

    exit(EXIT_SUCCESS);
}