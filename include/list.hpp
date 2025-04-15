//
// Created by jerry on 2024/9/5.
//

#ifndef DATASTRUCTURE_LIST_H
#define DATASTRUCTURE_LIST_H
#include <iostream>
#include <cassert>

namespace mycode{

    template <class T>
    struct ListNode{
        typedef ListNode<T> Node;
        typedef ListNode<T>& Ref;
        typedef ListNode<T>* Ptr;
        explicit ListNode(const T& val,Ptr next = nullptr,Ptr prev = nullptr):_val(val),_next(next),_prev(prev)
        {}
        explicit ListNode(T&& val,Ptr next = nullptr,Ptr prev = nullptr):_val(std::forward<T>(val)),_next(next),_prev(prev)
        {}
        T _val;
        Ptr _next;
        Ptr _prev;
    };
    //这里还是得重构一下Iterator
    template <class T,class Ref,class Ptr>
    struct ListIterator{
        typedef ListNode<T> Node;
        typedef ListIterator<T,Ref,Ptr> Self;
        typedef ListIterator<T,T&,T*> iterator;
        typedef ListIterator<T,const T&,const T*> const_iterator;
        ListIterator(Node* cur):_cur(cur){}
        //本次我的改动主要是这里的问题,就是这里的拷贝构造和复制重载函数其实不必要的并且可能会成为我这里的拖累
  
        ListIterator(const iterator& it):_cur(it._cur){}//这个函数的功能其实是const_iterator rit = it(iterator)
        //同样这里的复制重载函数也同样的需要解决
        Self& operator=(const iterator& it){
            _cur = it._cur;
            return *this;
        }
//        ListIterator(const Self& self):_cur(self._cur){}
//        Self& operator=(const Self& self){
//            _cur = self._cur;
//            return *this;
//        }
//        template <class U,class R,class P>
//        Self& operator=(const ListIterator<U,R,P>& self){
//            _cur = self._cur;
//            return *this;
//        }
//        template <class U,class R,class P>
//        explicit ListIterator(const ListIterator<U,R,P>& self):_cur(self._cur){}//这里其实是保证了iterator -> 构造const_iterator
        Ptr operator->() { return &(_cur->_val); }
        Ref operator*() { return _cur->_val; }
        Self operator+(int i){
            if(i >= 0){
                while(i--) ++(*this);
                return *this;
            }else {
                i = -i;
                while(i--) --(*this);
                return *this;
            }
        }
        Self operator-(int i){
            return (*this) + (-i);
        }
        Self& operator++(){
            _cur = _cur->_next;
            return *this;
        }
        Self operator++(int){
            Self tmp(_cur);
            _cur = _cur->_next;
            return tmp;
        }
        Self& operator--(){
            _cur = _cur->_prev;
            return *this;
        }
        Self operator--(int){
            Self tmp(_cur);
            _cur = _cur->_prev;
            return tmp;
        }
        bool operator!=(const Self& self) { return _cur != self._cur; }
        bool operator==(const Self& self) { return _cur == self._cur; }
        Node* _cur;
    };
    template <class T>
    class list{
    public:

        typedef ListNode<T> Node;
        typedef ListNode<T>* Ptr;
        typedef ListNode<T>& Ref;
        typedef list<T>& Self;
        //构造函数
        explicit list() = default;
        // typedef ListIterator<T,T&,T*> iterator;
        // typedef ListIterator<T,const T&,const T*> const_iterator;
        using iterator = ListIterator<T,T&,T*>;
        using const_iterator = ListIterator<T,const T&,const T*>;
        iterator begin() { return iterator(_head); }
        const_iterator begin() const { return const_iterator(_head); }
        iterator end() { return iterator(m_head); }
        const_iterator end() const { return const_iterator(m_head); }
        list(const std::initializer_list<T>& il){
            for(auto& e : il)
                push_back(e);
        }
        //list的构造函数 和 复制重载函数
        template <class InputIterator>
        explicit list(InputIterator begin,InputIterator end){
            while(begin != end)
                push_back(*(begin++));
        }
        explicit list(const list<T>& li){
            for(auto& e : li)
                push_back(e);
        }
        explicit list(list<T>&& li){
            std::swap(_head,li._head);
            std::swap(_tail,li._tail);
            std::swap(_size,li._size);
        }
        list<T>& operator=(const list<T>& li){
            for(auto& e : li)
                push_back(e);
            return *this;
        }
        list<T>& operator=(list<T>&& li){
            std::swap(_head,li._head);
            std::swap(_tail,li._tail);
            std::swap(_size,li._size);
            return *this;
        }
        //
        void swap(list<T>& li){
            std::swap(_head,li._head);
            std::swap(_tail,li._tail);
            std::swap(_size,li._size);
        }
        void push_back(const T& val);
        void push_back(T&& val);
        bool empty() const { return _head == nullptr; }
        size_t size() const { return _size; }
        class poly{
            explicit poly(bool flag,Node* ptr):_flag(flag),_position(ptr)
            {}
            operator bool(){ return _flag; } // 本来poly就是为了insert 和 erase服务的, 因此不用使用const 对类型函数的转换进行修饰
            operator iterator(){ return _position; }
            bool _flag;
            mycode::ListIterator<T,T&,T*> _position;
        };
        template <class Elem>
        poly insert(Elem&& val,iterator pos){
            Node* newnode = new Node(std::forward<Elem>(val));
            if(newnode == nullptr) return poly(false,nullptr);
            iterator it = begin();
            while(it != pos)
                ++it;
            Node* prev = it->_cur->_prev;
            Node* cur = it->_cur;
            prev->_next = newnode; newnode->_prev = prev; 
            newnode->_next = cur; newnode->_next = cur;
            return poly(true,newnode);
        }
        void pop_back();
        T& back() { return _tail->_val; }
        const T& back() const { return _tail->_val; }
        T& front() { return _head->_val; }
        const T& front() const { return _head->_val; }
        template <class ...Args>//这里是同样的是效仿了stl中提出的emplace_back的接口
                void emplace_back(Args... args){
            Ptr newnode = Node(args...);
            if(_head == nullptr) {
                _head = _tail = newnode;
                _head->_prev = _head->_next = m_head;
                m_head->_prev = m_head->_next = newnode;
            }
            else {
                _tail->_next = newnode;
                newnode->_prev = _tail;
                newnode->_next = m_head;
                m_head->_prev = newnode;
                _tail = newnode;
            }
            _size++;
        }
    private:
        //带头双向链表
        Ptr m_head = new Node(T());
        Ptr _head = nullptr;
        Ptr _tail = nullptr;
        size_t _size = 0;
    };

    template <class T>
    void list<T>::push_back(const T& val){
        Ptr newnode = new Node(val);
        if(_head == nullptr) {
            _head = _tail = newnode;
            _head->_prev = _head->_next = m_head;
            m_head->_prev = m_head->_next = newnode;
        }
        else {
            _tail->_next = newnode;
            newnode->_prev = _tail;
            newnode->_next = m_head;
            m_head->_prev = newnode;
            _tail = newnode;
        }
        _size++;
    }
    template <class T>
    void list<T>::push_back(T&& val){
        Ptr newnode = new ListNode<T>(std::forward<T>(val));
        if(_head == nullptr) {
            _head = _tail = newnode;
            _head->_prev = _head->_next = m_head;
            m_head->_prev = m_head->_next = newnode;
        }
        else {
            _tail->_next = newnode;
            newnode->_prev = _tail;
            newnode->_next = m_head;
            m_head->_prev = newnode;
            _tail = newnode;
        }
        _size++;
    }
    template <class T>
    void list<T>::pop_back(){
        assert(_size--);
        Ptr del = _tail;
        if(_head == _tail){
            //说明只有一个节点
            _head = _tail = nullptr;
            m_head->_prev = m_head->_next = nullptr;
        }else{
            _tail = _tail->_prev;
            _tail->_next = m_head;
            m_head->_prev = _tail;
        }
        delete del;
    }
}
namespace StandardModule
{
    template <class T>
    using list = mycode::list<T>;
}
#endif //DATASTRUCTURE_LIST_H
