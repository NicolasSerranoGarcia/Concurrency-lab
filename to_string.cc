#include <iostream>
#include <string>
#include <algorithm>


std::string to_string_single(unsigned int num){
    char c = num + 48;
    return std::string{c};
}

std::string to_string(unsigned int num){
    if(num == 0){
        return std::string("0");
    }

    std::string s;

    for(unsigned int i = num; i != 0; i/=10){
        s += to_string_single(i % 10);
    }

    std::string::npos;

    std::reverse(s.begin(), s.end());

    return s;

}


int main(){
    std::cout << to_string(2245) << std::endl;
}
