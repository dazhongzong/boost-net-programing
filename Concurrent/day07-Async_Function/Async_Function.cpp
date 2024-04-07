﻿
#include <algorithm>
#include <iostream>
#include <list>

//c++ 版本的快速排序算法
template<typename T>
void quick_sort_recursive(T arr[], int start, int end) {
    if (start >= end) return;
    T key = arr[start];
    int left = start, right = end;
    while (left < right) {
        while (arr[right] >= key && left < right) right--;
        while (arr[left] <= key && left < right) left++;
        std::swap(arr[left], arr[right]);
    }

    if (arr[left] < key) {
        std::swap(arr[left], arr[start]);
    }


    quick_sort_recursive(arr, start, left - 1);
    quick_sort_recursive(arr, left + 1, end);
}
template<typename T>
void quick_sort(T arr[], int len) {
    quick_sort_recursive(arr, 0, len - 1);
}

//函数式编程
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
    if (input.empty())
    {
        return input;
    }
    std::list<T> result;

    //  ① 将input中的第一个元素放入result中，并且将这第一个元素从input中删除
    result.splice(result.begin(), input, input.begin());

    //  ② 取result的第一个元素，将来用这个元素做切割，切割input中的列表。
    T const& pivot = *result.begin();

    //  ③std::partition 是一个标准库函数，用于将容器或数组中的元素按照指定的条件进行分区，
    // 使得满足条件的元素排在不满足条件的元素之前。
    // 所以经过计算divide_point指向的是input中第一个大于等于pivot的元素
    auto divide_point = std::partition(input.begin(), input.end(),
        [&](T const& t) {return t < pivot; });

    // ④ 我们将小于pivot的元素放入lower_part中
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(),
        divide_point);

    // ⑤我们将lower_part传递给sequential_quick_sort 返回一个新的有序的从小到大的序列
    //lower_part 中都是小于divide_point的值
    auto new_lower(
        sequential_quick_sort(std::move(lower_part)));
    // ⑥我们剩余的input列表传递给sequential_quick_sort递归调用，input中都是大于divide_point的值。
    auto new_higher(
        sequential_quick_sort(std::move(input)));
    //⑦到此时new_higher和new_lower都是从小到大排序好的列表
    //将new_higher 拼接到result的尾部
    result.splice(result.end(), new_higher);
    //将new_lower 拼接到result的头部
    result.splice(result.begin(), new_lower);
    return result;
}

int main()
{
    std::cout << "Hello World!\n";
}


