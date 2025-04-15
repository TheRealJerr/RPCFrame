#pragma once
#include "vector.hpp"
#include "list.hpp"
#include "priority_queue.hpp"

// 写一个deque也可以, 但是我本身只知道deque的逻辑,没有实现过,所以这里的queue默认的list

namespace mycode
{
    template <class T,class Container = mycode::list<T>>
    class queue
    {
    public:
        using iterator = typename Container::iterator;
        using const_iterator = typename Container::const_iterator;

        void push(const T& val) { _con.push_back(val); }
        void push(T&& val) { _con.push_back(std::move(val)); }
        void pop() { _con.erase(_con.begin()); }

        template <class ...Args>
        void emplace(Args&&... args) { _con.emplace_back(std::forward<Args>(args)...); }

        size_t size() const { return _con.size(); }
        size_t capacity() const { return _con.capacity(); }

        T& front() { return _con.front(); }
        const T& front() const { return _con.front(); }

        // iterator

        iterator begin() { return _con.begin(); }
        const_iterator begin() const { return _con.begin(); }

        iterator end() { return _con.end(); }
        const_iterator end() const { return _con.end(); }

    private:
        Container _con;
    };
    
}
namespace StandardModule
{
    template <class T>
    using queue = mycode::queue<T>;
}