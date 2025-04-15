//
// Created by jerry on 2024/8/11.
//

#ifndef VECTOR_H
#define VECTOR_H
#include <iostream>
using std::cin;
using std::size_t;
namespace mycode {

    using std::size_t;
    template <class T,class Ref,class Ptr>
    struct vectorIterator{
        typedef vectorIterator<T,Ref,Ptr> self;
        typedef vectorIterator<T,Ref,Ptr>& ref;
        typedef vectorIterator<T,Ref,Ptr>* ptr;
        using iterator = vectorIterator<T,T&,T*>;
        using const_iterator = vectorIterator<T,const T&,const T*>;
//        vectorIterator(const vectorIterator<T>& it):_node(it._node){}
        vectorIterator(T* it_ptr):_node(it_ptr){}
//        ref operator=(const vectorIterator<T>& it){
//            _node = it._node;
//            return *this;
//        }
        vectorIterator(const iterator& it):_node(it._node){}
        ref operator=(const iterator& it){
            _node = it._node;
            return *this;
        }
        T& operator*(){ return *_node; }
        T* operator->(){ return _node; }
        ///////
        ref operator++(){
            _node++;
            return *this;
        }
        self operator++(int){
            self tmp(_node);
            _node++;
            return tmp;
        }
        ref operator--(){
            _node--;
            return *this;
        }
        self operator--(int){
            self tmp(_node);
            _node--;
            return tmp;
        }
        self operator+(int len){
            return self(_node + len);
        }
        self operator-(int len){
            return self(_node-len);
        }
        template <class It>
        bool operator!=(It it){
            return _node != it._node;
        }
        template<class It>
                int operator-(It it){
            return _node - it._node;
        }
        T* _node;
    };
//
//    template <class T>
//    struct vectorConstIterator{
//
//        typedef vectorConstIterator<T> self;
//        typedef vectorConstIterator<T>& ref;
//        typedef vectorConstIterator<T>* ptr;
//        vectorConstIterator(const T* it_ptr):_node(it_ptr){}
//        vectorConstIterator(const vectorConstIterator<T>& it):_node(it._node){}
//        ref operator=(const vectorConstIterator<T>& it){
//            _node = it._node;
//            return *this;
//        }
//        vectorConstIterator(const vectorIterator<T>& it):_node(it._node){}
//        ref operator=(const vectorIterator<T>& it){
//            _node = it._node;
//            return *this;
//        }
//        const T& operator*(){ return *_node; }
//        const T* operator->(){ return _node; }
//        ///////
//        ref operator++(){
//            _node++;
//            return *this;
//        }
//        self operator++(int){
//            self tmp(_node);
//            _node++;
//            return tmp;
//        }
//        ref operator--(){
//            _node--;
//            return *this;
//        }
//        self operator--(int){
//            self tmp(_node);
//            _node--;
//            return tmp;
//        }
//        self operator+(int len){
//            return self(_node + len);
//        }
//        self operator-(int len){
//            return self(_node-len);
//        }
//        template <class It>
//        bool operator!=(It it){
//            return _node != it._node;
//        }
//        template<class It>
//                int operator-(It it){
//            return _node - it._node;
//        }
//        const T* _node;
//    };

    template <class T>
    class vector {
    public:
        //默认构造,拷贝构造,移动构造,赋值重载,移动重载函数
        explicit vector() = default;
        vector(const std::initializer_list<T>& il) { for(auto& e : il) push_back(e); }
        explicit vector(size_t size) { resize(size); }
        template <class InputIterator>
        explicit vector(InputIterator begin,InputIterator end){
            while(begin != end){
                push_back(*begin);
                begin++;
            }
        }


        vector(const vector<T>& self)
        {
            reliese();
            _head = new T[self._size];
            for(int i = 0;i < self._size;i++)
                _head[i] = self._head[i];
            _size = self._size;
            _capacity = self._capacity;
        }
        vector(vector<T>&& self){
            std::swap(_head,self._head);
            std::swap(_size,self._size);
            std::swap(_capacity,self._capacity);
        }
        vector<T>& operator=(vector<T> self){//现代写法
            std::swap(_head,self._head);
            std::swap(_size,self._size);
            std::swap(_capacity,self._capacity);
            return *this;
        }
        vector<T>& operator=(vector<T>&& self){
            std::swap(_head,self._head);
            std::swap(_size,self._size);
            std::swap(_capacity,self._capacity);
            return *this;
        }
        //迭代器的实现
        using iterator = vectorIterator<T,T&,T*>;
        using const_iterator = vectorIterator<T,const T&,const T*>;
        iterator begin() { return iterator(_head); }
        iterator end() { return iterator(_head + _size); }
        const_iterator begin() const { return const_iterator(_head); }
        const_iterator end() const { return const_iterator(_head + _size); }
        ///////////////////////////////////////////
        //push和pop函数
        void push_back(const T& val);
        void push_back(T&& val);
        void pop_back() { _size--; }
        //insert ,erase函数
        struct poly{
            poly(int pos,vector<T>* whole):_pos(pos),_whole(whole)
            {}
            operator bool() const{
                if(_pos < 0 || _pos >= _whole->_size) return true;
                else return false;
            }
            operator vector<T>::iterator() const{
                return iterator(const_cast<T*>(_whole->_head) + _pos);
            };
            int _pos;
            vector<T>* _whole;
        };
        template <class I>
        poly insert(iterator pos,I&& input){
            int src = pos - begin();
            if(_size == _capacity) recapacity(_size * 2);
            int end = size();
            while(end > src)
                _head[end] = _head[--end];
            _head[src] = std::forward<I>(input);
            _size++;
            return poly(src,this);
        }
        poly erase(iterator pos){
            int src = pos - begin();
            int begin = src;
            while(src < end() - 1)
                _head[src] = _head[++src];
            _size--;
            return poly(src,this);
        }
        poly erase(int size){
            int src = size;
            int begin = src;
            while(src < end() - 1)
                _head[src] = _head[++src];
            _size--;
            return poly(src,this);
        }

        //我们设计一个返回的类专门用于进行返回值的类型的转换
        size_t size() const { return _size; }
        size_t capacity() const { return _capacity; };
        void resize(size_t newSize);
        void recapacity(size_t newCapacity);
        T& back() { return *(_head + _size - 1); }
        const T& back() const { return *(_head + _size - 1); }
        T& front() { return *_head; }
        const T& front() const { return *_head; }

        T& operator[](size_t size) { return _head[size]; }
        const T& operator[](size_t size) const { return _head[size]; }
        bool empty() const { return _head == nullptr; }
        void swap(vector<T>& rv){
            std::swap(_head,rv._head);
            std::swap(_size,rv._size);
            std::swap(_capacity,rv._capacity);
        }
        void reliese(){
            delete[]_head;
            _head = nullptr;
            _size = _capacity = 0;
        }
        ~vector() { reliese(); }
        template <class ...Args>
                void emplace_back(Args&&... args)
        {
            if(_size == _capacity) recapacity(_capacity *  2);
            _head[_size++] = T(std::forward<Args>(args)...);
        }
    private:
        T* _head = new T[4];
        size_t _size = 0;
        size_t _capacity = 4;//默认给四个位置
    };
    template <class T>
    void vector<T>::recapacity(size_t newCapacity)
    {
        if(newCapacity > _capacity)
        {
            T* newHead = new T[newCapacity];
            for(size_t i = 0;i < _size;i++)
                newHead[i] = _head[i];
            delete[]_head;
            _head = newHead;
            _capacity = newCapacity;
        }
    }
    template <class T>
    void vector<T>::resize(size_t newSize)
    {
        recapacity(newSize);
        _size = newSize;
    }
    template <class T>
    void vector<T>::push_back(const T& val)
    {
        if(_size == _capacity) recapacity(_capacity * 2);
        _head[_size++] = val;
    }
    template <class T>
    void vector<T>::push_back(T&& val)
    {
        if(_size == _capacity) recapacity(_capacity * 2);
        _head[_size++] = (std::forward<T>(val));//这里升级了右值引用
    }

    
}
namespace StandardModule
{
    template <class T>
    using vector = mycode::vector<T>;
}
#endif //VECTOR_H
