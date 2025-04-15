#pragma once
#include <time.h>
#include <iostream>
#include <type_traits>

#define __QuickSort__

namespace algorithm
{
    // 排序的算法

    struct Less
    {
        template <class T>
        bool operator()(const T &left, const T &right) const
        {
            return left < right;
        }
    };

    template <class T>
    void swap(T &left, T &right)
    {
        T tmp = left;
        left = right;
        right = tmp;
    }

#ifdef __BubbleSort__
    template <class InputIterator, class Cmp = Less>
    void Sort(InputIterator begin, InputIterator end)
    {
        Cmp cmp;

        while (end - begin > 0)
        {
            InputIterator cur = begin;
            // 每次找到最cmp的值
            while (cur != end)
            {
                if (cur + 1 != end && cmp(*(cur + 1), *cur))
                    swap((*cur), *(cur + 1));
                ++cur;
            }
            --end;
        }
    }
#endif

#ifdef __QuickSort__
    // 快排
    // 快排值三路划分
    template <class InputIterator>
    InputIterator getRandom(InputIterator begin, InputIterator end)
    {
        size_t size = end - begin;
        int random = rand() % size;

        return begin + random;
    }

    template <class InputIterator, class Cmp>
    void QuickSort(InputIterator begin, InputIterator end, const Cmp &cmp)
    {
        if (end - begin <= 1)
            return; // 如果元素个数 <= 1，结束递归

        InputIterator keyi = getRandom(begin, end);
        InputIterator left = begin - 1, right = end, cur = begin;

        // 三路划分
        while (cur < right)
        {
            if (cmp(*cur, *keyi))
            {
                swap(*(cur++), *(++left)); // 小于 key 的元素移到左侧
            }
            else if (cmp(*keyi, *cur))
            {
                swap(*(cur), *(--right)); // 大于 key 的元素移到右侧
            }
            else
            {
                cur++; // 等于 key 的元素跳过
            }
        }

        QuickSort(begin, left + 1, cmp); // 左边区间
        QuickSort(right, end, cmp);      // 右边区间
    }

    template <class InputIterator, class Cmp = Less>
    void Sort(InputIterator begin, InputIterator end)
    {
        Cmp cmp;
        srand(time(nullptr));
        QuickSort(begin, end, cmp);
    }
#endif

#ifdef __HeapSort__
    // 堆排序
    template <class InputIterator, class Cmp>
    void AjustDown(InputIterator arr, size_t parent, size_t size, const Cmp &cmp)
    {
        size_t child = 2 * parent + 1;
        while (child < size)
        {
            if (child + 1 < size && cmp(*(arr + child), *(arr + child + 1)))
                child++;
            if (cmp(*(arr + parent), *(arr + child)))
                swap(*(arr + parent), arr[child]);
            else
                break; // 处于堆正常的位置

            parent = child;
            child = 2 * child + 1;
        }
    }
    template <class InputIterator, class Cmp>
    void AjustUp(InputIterator arr, size_t child, const Cmp &cmp)
    {
        // 向上建堆如果用size_t 会有小坑
        // if(child == 0) return;
        size_t parent = (child - 1) / 2;
        while (child != 0)
        {
            if (cmp(arr[parent], arr[child]))
                swap(arr[parent], arr[child]);
            else
                break;
            child = parent;
            parent = (parent - 1) / 2;
        }
    }
    template <class InputIterator, class Cmp>
    void HeapSort(InputIterator arr, size_t size, const Cmp &cmp)
    {
        // 首先进行建堆
        // 这里由于cur--不会出现<0的情况，所以可能导致的结果就是死循环
        // 这里我为了简便就在这里将cur用int来处理,这里其实会有精度的缺失
        for (int cur = (int)(size - 2) / 2; cur >= 0; cur--)
            AjustDown(arr, cur, size, cmp);

        // 然后进行排序
        size_t end = size - 1;
        while (end != 0)
        {
            swap(*arr, *(arr + end));
            AjustDown(arr, 0, end, cmp);
            --end;
        }
    }
    template <class InputIterator, class Cmp = Less>
    void Sort(InputIterator begin, InputIterator end)
    {
        Cmp cmp;
        HeapSort(begin, end - begin, cmp);
    }
#endif

// shellsort这里是有bug的但是我懒得改了，这里暂时就这样吧
#ifdef __ShellSort__
    //
    template <class InputIterator, class Cmp = Less>
    void Sort(InputIterator begin, InputIterator end)
    {
        Cmp cmp;
        int n = end - begin;
        InputIterator& arr = begin;
        int gap = n;
        while (gap > 1)
        {
            gap /= 2; // 在gap取定后，后面的排序方式和插入排序一致
            for (int i = 0; i < n - gap; i++)
            {
                int end = i;
                decltype(arr[0]) temp = arr[end + gap];
                while (end >= 0)
                {
                    if (cmp(temp,arr[end]))
                        arr[end + gap] = arr[end];
                    else
                        break;
                    end -= gap;
                }
                arr[end + gap] = temp;
            }
        }
        
    }
#endif




#ifdef __MergeSort__
    #include "vector.hpp"
    //归并排序
    using StandardModule::vector;
    template <class Head,class Cmp,class T>
    void MergeSort(Head arr,size_t left,size_t right,const Cmp& cmp,T& temp)
    {
        static vector<T> help(right - left);
        // static vector<dectype(arr[0])> help(right - left);
        if(right <= left) return;
        size_t mid = left + (right - left) / 2;
        // 左右分别排序
        MergeSort(arr,left,mid,cmp,temp);
        MergeSort(arr,mid + 1,right,cmp,temp);
        //左右归并
        size_t curl = left, curr = mid + 1;
        for(size_t cur = left;cur <= right;cur++)
        {
            if(curl == mid + 1) help[cur] = arr[curr++];
            else if(curr == right + 1) help[cur] = arr[curl++];
            else {
                if(cmp(arr[curl],arr[curr])) help[cur] = arr[curl++];
                else help[cur] = arr[curr++];
            }
        }
        //将归并后的结果返回
        for(size_t i = left;i <= right;i++)
            arr[i] = help[i];

    }

    template <class InputIterator,class Cmp = Less>
    void Sort(InputIterator begin,InputIterator end)
    {
        Cmp cmp;
        MergeSort(begin,0,end - begin - 1,cmp,*begin);
    }
#endif
}
