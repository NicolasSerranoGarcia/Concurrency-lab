#include <numeric>
#include <vector>
#include <iostream>
#include <thread>


template<typename Iterator, typename Init>
struct accumulate_block{
    void operator()(Iterator start, Iterator end, Init& init){
        init = std::accumulate(start, end, init);
    };
};

template<typename Iterator, typename Init>
Init parallelAccumulate(Iterator start, Iterator end, Init init){
    //this should leave the processing of the total list of values divided into threads. 
    //For example, if the start-end = 25, then you might create 5 threads, and non-concurrently assign the processing of
    //each chunk of 5 to a thread. Each will call accumulate_block, with the reduced iterators, instead of sequentially processing
    //each of the 25, at the end we will have a total of 5 smaller results, computed in parallel which then can be summed up non-concurrently. 
    //If we assume 0 overhead for anything else than the actual computation, and use X number of threads to manage the whole dataset, the task 
    //will be completed in X times faster than doing it linearly. If linearly it takes 10 seconds, in parallel it takes 2 seconds (assuming summing
    //the remaining 5 results takes 0 units of time)

    //can return < 0, but only on random access iterators (which is the case for vectors)
    const unsigned long long int numData = std::distance(start, end);

    if(!numData){
        return init;
    }


    const unsigned long int minDataPerThread = 25;

    //it must be at least one, so move the result by 1
    const unsigned long int allThreadsRequiredInTheory = 1 + (numData / minDataPerThread);

    const unsigned long int maxThreadsAvailableOnHardware = std::thread::hardware_concurrency();

    //in order to choose a reasonable amount of threads to complete the task, we take the minimum between the number of maximum threads
    //we would need for maximum distribution of data and the number of real threads
    
    const unsigned long int numThreadsWhichWeAreGonnaUse = std::min(maxThreadsAvailableOnHardware != 0 ? maxThreadsAvailableOnHardware : 2, allThreadsRequiredInTheory);

    //note that the main thread counts as one already, so we need one less
    std::vector<Init> blockResults(numThreadsWhichWeAreGonnaUse);

    std::vector<std::thread> threads(numThreadsWhichWeAreGonnaUse - 1);

    //now, we need to choose the iterators we are gonna pass to each one of the threads

    Iterator it = start;

    for(unsigned int i = 0; i < numThreadsWhichWeAreGonnaUse - 1; i++){

        auto itEnd = it;

        std::advance(itEnd, minDataPerThread);

        threads[i] = std::thread{accumulate_block<Iterator, Init>(), it, itEnd, std::ref(blockResults[i])};
        it = itEnd;
    }

    //as inside the for loop we have filled until the last one, leaving the last one without a value, the main thread is the one in charge of executing the last chunk
    accumulate_block<Iterator, Init>{}(it, end, std::ref(blockResults[numThreadsWhichWeAreGonnaUse - 1]));


    for(unsigned int i = 0; i < numThreadsWhichWeAreGonnaUse - 1; i++){
        threads[i].join();
    }


    return std::accumulate(blockResults.begin(), blockResults.end(), init);
};


int main(){

    std::vector<int> vec = {5,4,3,2,2,4,5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,33,3,34345,345,435,453,45345,345,345,345,345,34,535,34,534534,534,53,45,345,45,345,435,345,34,534,534,};

    unsigned long long res = parallelAccumulate(vec.begin(), vec.end(), 0);

    std::cout << res << std::endl;

    return 0;
}