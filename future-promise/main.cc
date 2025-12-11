#include "myfuture.h"
#include "mypromise.h"
#include "async.h"
#include <thread>
#include <iostream>
#include <string>

void print(std::string s){
    std::cout << s << std::endl;
}

int main(){

    std::string s("Hello World!");

    return 0;
}