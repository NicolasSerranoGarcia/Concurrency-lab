#include <algorithm>
#include <list>
#include <future>

template<typename T>
inline std::list<T> quick_sort_sequential(std::list<T> list){

    if(list.empty()){
        return list;
    }

    std::list<T> res;

    //put on res.begin(), the list.begin() element of list. For the exercise, this means moving the first element
    //of list into res, as res is empty. This is used as the pivot
    res.splice(res.begin(), list, list.begin());

    //read only value of the pivot. Note the dereferencing of the iterator
    const T& pivot = *res.begin();

    //rearrange list into lower and higher part. The elements lower than the pivot will be at the start.
    //The elements higher than pivot will be at the end. The turning point is returned as an iterator. 
    //So dividePoint is the first element where the condition is not true (the first element that >= pivot)
    auto dividePoint = std::partition(list.begin(), list.end(), [&](const T& t){ return t < pivot; });

    std::list<T> lower;

    //Take the lower part of the list (the elements lower than the pivot), and move them into the lower list
    //(here lower is empty, so it doesn't matter if we specify lower.begin() or lower.end(), as we are moving the items to the empty array)
    lower.splice(lower.begin(), list, list.begin(), dividePoint);

    //divide and conquer
    std::list<T> lowerRes = quick_sort_sequential(std::move(lower));
    std::list<T> higherRes = quick_sort_sequential(list);

    //as the pivot is already in the list, insert before and after it 
    res.splice(res.begin(), lowerRes);
    res.splice(res.end(), higherRes);

    return res;
}


template<typename T>
inline std::list<T> quick_sort_parallel(std::list<T> list){

    if(list.empty()){
        return list;
    }

    std::list<T> res;

    //put on res.begin(), the list.begin() element of list. For the exercise, this means moving the first element
    //of list into res, as res is empty. This is used as the pivot
    res.splice(res.begin(), list, list.begin());

    //read only value of the pivot. Note the dereferencing of the iterator
    const T& pivot = *res.begin();

    //rearrange list into lower and higher part. The elements lower than the pivot will be at the start.
    //The elements higher than pivot will be at the end. The turning point is returned as an iterator. 
    //So dividePoint is the first element where the condition is not true (the first element that >= pivot)
    auto dividePoint = std::partition(list.begin(), list.end(), [&](const T& t){ return t < pivot; });

    std::list<T> lower;

    //Take the lower part of the list (the elements lower than the pivot), and move them into the lower list
    //(here lower is empty, so it doesn't matter if we specify lower.begin() or lower.end(), as we are moving the items to the empty array)
    lower.splice(lower.begin(), list, list.begin(), dividePoint);

    //divide and conquer
    std::future<std::list<T>> lowerRes(std::async(&quick_sort_parallel<T>, std::move(lower)));
    std::list<T> higherRes = quick_sort_parallel(list);

    //as the pivot is already in the list, insert before and after it 
    res.splice(res.begin(), lowerRes.get());
    res.splice(res.end(), higherRes);
    
    return res;
}