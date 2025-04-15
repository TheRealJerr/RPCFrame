//
// Created by jerry on 2024/10/11.
//
#include "vector.hpp"
#include "list.hpp"
#ifndef DATASTRUCTURE_STACK_H
#define DATASTRUCTURE_STACK_H
namespace mycode
{
    template <class T, class Container = mycode::list<T>>
    class stack
    {
    public:
        using Self = stack<T, Container>;
        using Ref = stack<T, Container> &;
        using Ptr = stack<T, Container> *;
        using iterator = typename Container::iterator;
        using const_iterator = typename Container::const_iterator;
        bool empty() const { return _con.empty(); }
        size_t size() const { return _con.size(); }

        explicit stack() = default;
        explicit stack(const Ref sk) : _con(sk._con)
        {
        }
        explicit stack(stack<T, Container> &&sk)
            : _con(std::move(sk._con))
        {
        }
        Ref operator=(const Ref sk)
        {
            _con = sk._con;
            return *this;
        }
        Ref operator=(stack<T, Container> &&sk)
        {
            _con = std::move(sk._con);
            return *this;
        }
        template <class Elem>
        void push(Elem &&val) { _con.push_back(std::forward<Elem>(val)); }
        void pop()
        {
            _con.pop_back();
        }
        T &top() { return _con.back(); }
        const T &top() const { return _con.back(); }
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
    using stack = mycode::stack<T>;
}
#endif // DATASTRUCTURE_STACK_H
