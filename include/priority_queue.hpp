#ifndef __priority_queue__
#define __priority_queue__

#include <iostream>
#ifndef BASESIZE
#define BASESIZE 4
#endif
namespace mycode
{

    // 通过数组实现的优先级队列 , 建立大小堆的方式
    template <class T, class Compare>
    class priority_queue
    {
    public:
        template <class InputIterator>
        priority_queue(InputIterator begin, InputIterator end);
        priority_queue() = default;

        // this part 我想要实现的还是通过数组建立的堆结构
        template <class Elem>
        void push(Elem &&val);
        void pop();

        // c++11
        template <class... Args>
        void emplace(Args &&...args);

        T &top() { return _head[0]; }
        const T &top() const { return _head[0]; }

        /// 调用的函数的集合
        void AjustDown(T *head, int parent, int size);
        void AjustUp(T *head, int child, int size);

        // 二倍扩容
        void expand(int ep_num = 0);

        // 释放空间
        void release();

    private:
        T *_head = nullptr;
        int _size = 0;
        int _capacity = 0;
        Compare _cmp;
    };

    template <class T, class Compare>
    void priority_queue<T, Compare>::expand(int ep_num)
    {
        // default operation
        if (ep_num == 0)
        {
            if (_head == nullptr)
            {
                _head = new T[BASESIZE];
                _capacity = BASESIZE;
            }
            else
            {
                T *newhead = new T[_capacity * 2];
                for (int i = 0; i < _capacity; i++)
                    newhead[i] = std::move(_head[i]);
                release();
                _head = newhead;
                _capacity *= 2;
            }
        }
        // user_defined operation
        else if (ep_num <= _capacity)
            return;
        else
        {
            T *newhead = new T[ep_num];
            for (int i = 0; i < _capacity; i++)
                newhead[i] = std::move(_head[i]);
            release();
            _head = newhead;
            _capacity = ep_num;
        }
    }

    template <class T, class Compare>
    void priority_queue<T, Compare>::release()
    {
        delete[] _head;
        _size = _capacity = 0;
    }

    template <class T, class Compare>
    void priority_queue<T, Compare>::AjustDown(T *head, int parent, int size)
    {
        // 向下调整法
        int child = parent * 2 + 1;
        while (child < size)
        {
            if (child < size && _cmp(head[child + 1], head[child]))
                child++;
            if (_cmp(head[child], head[parent]))
                std::swap(_head[child], _head[parent]);
            else
                break;
            parent = child;
            child = child * 2 + 1;
        }
    }

    template <class T, class Compare>
    void priority_queue<T, Compare>::AjustUp(T *head, int child, int size)
    {
        int parent = (child - 1) / 2; // just for c++
        // 由于c++的特性 0 - 1 / 2 = 0 所有这里会陷入一个死循环
        if (child == 0)
            return;
        while (parent >= 0)
        {
            if (_cmp(head[child], head[parent]))
                std::swap(head[child], head[parent]);
            else
                break;
            child = parent;
            parent = child == 0 ? -1 : (child - 1) / 2;
        }
    }

    template <class T, class Compare>
    template <class InputIterator>
    priority_queue<T, Compare>::priority_queue(InputIterator begin, InputIterator end)
    {
        // to get the input container size
        int size = 0;
        InputIterator cur = begin;
        while (cur++ != end)
            size++;

        expand(size);

        cur = begin;
        for (int i = 0; i < size; i++)
            _head[i] = *(cur++);
        for (int end = (size - 2) / 2; end >= 0; end--)
            AjustDown(_head, end, size);
        _size = size;
    }

    template <class T, class Compare>
    template <class Elem>
    void priority_queue<T, Compare>::push(Elem &&val)
    {
        if (_size == _capacity)
            expand();
        _head[_size++] = std::forward<Elem>(val);
        AjustUp(_head, _size - 1, _size);
    }

    template <class T, class Compare>
    template <class... Args>
    void priority_queue<T, Compare>::emplace(Args &&...args)
    {
        if (_size == _capacity)
            expand();
        _head[_size++] = T(std::forward<Args>(args)...);
        AjustUp(_head, _size - 1, _size);
    }

    template <class T, class Compare>
    void priority_queue<T, Compare>::pop()
    {
        if (_size == 0)
            return;
        std::swap(_head[0], _head[_size - 1]);
        _size--;
        AjustDown(_head, 0, _size);
    }

}
namespace StandardModule
{
    template <class T, class Compare = std::less<T>>
    using priority_queue = mycode::priority_queue<T, Compare>;
}
#endif