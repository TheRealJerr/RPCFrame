#pragma once
#include <iostream>
#include <cstring>
#include <ctype.h>
#include "vector.hpp"
#include <unistd.h>

namespace mycode
{
#define NPOS INT32_MAX
    // iterator block
    using StandardModule::vector;
    template <class T>
    class _strIterator
    {
    private:
        using self = _strIterator<T>;
        using ref = _strIterator<T>&;
        using ptr = _strIterator<T>*;
    public:
        _strIterator(T* cur):_cur(cur) 
        {}

        _strIterator(const _strIterator<char>& other)
        {
            _cur = other._cur;
        }

        ref operator=(const _strIterator<char>& other)
        {
            _cur = other._cur;
            return *this;
        }

        T& operator*() { return *_cur; }
        T* operator->() { return _cur; }
        
        ref operator++() { 
            ++_cur;
            return *this;
        }
        self operator++(int)
        {
            self tmp()
        }

        ref operator--();
        ref operator--(int);
    private:
        T* _cur;
    };



    static int default_string_size = -1;
    class string
    {
    protected:
        // 这里定义kmp算法
        vector<int> getNextArray(const string& l,const string& r) const 
        {
            // 得到next数组
            vector<int> ret;
            int rpos = 0;
            for(int i = 0; i < l.size(); i++)
            {
                if(r[rpos] == l[i]) 
                    ret.push_back(++rpos);
                
                else if(rpos != 0)
                {
                    rpos = 0;
                    if(r[rpos] == l[i]);
                    ret.push_back(++rpos);
                }
            }
            return ret;
        }
        void expand(int ex_size = default_string_size)
        {
            // 默认是进行二倍扩容
            if (ex_size == default_string_size)
            {
                if (_head == nullptr)
                {
                    _head = new char[4];
                    _capacity = 4;
                }
                else
                {
                    char *newhead = new char[_capacity * 2 + 1];
                    strncpy(newhead, _head, sizeof(newhead));
                    _head[_capacity] = 0;
                    delete[] _head;
                    _head = newhead;
                    _capacity *= 2;
                }
            }
            else if (ex_size > _capacity)
            {
                if (_head == nullptr)
                    return;
                char *newhead = new char[ex_size + 1]{0};
                strncpy(newhead, _head, sizeof(newhead));
                _head[_capacity] = 0;
                delete[] _head;
                _capacity = ex_size;
            }
            else
                perror("expand size less than before");
        }

        void release()
        {
            delete[] _head;
            _size = _capacity = 0;
        }

    public:
        string() = default;
        string(const char *);
        string(const mycode::string &);
        string(mycode::string &&);

        // operator block
        string operator+(const mycode::string &);
        string &operator+=(const char *);
        string &operator+=(const mycode::string &);

        char& operator[](size_t pos);
        const char& operator[](size_t pos) const;

        void append(const string &);
        void append(char);

        void push_back(char ch);
        void pop_back();

        void clear();

        void swap(mycode::string&);

        const char *c_str() const;
        void copy(const string &);

        bool compare(const string &);

        size_t find(const string &) const;
        size_t find(char ch) const;

        string substr(size_t begin, size_t size);
        // string substr(iterator begin,iterator end);

        size_t size() const { return _size; }
        size_t capacity() const { return _capacity; }

        const char *c_str() const { return _head; }
        ~string();

    private:
        char *_head = nullptr;
        size_t _size = 0;
        size_t _capacity = 0;
    };

    char& string::operator[](size_t pos)
    {
        if(pos > _size)
        {
            std::cerr << "Segment Default[string]" << std::endl;
            ::exit(1);
        }
        return _head[pos];
    }
    
    const char& string::operator[](size_t pos) const 
    {
        if(pos > _size)
        {
            std::cerr << "Segment Default[string]" << std::endl;
            ::exit(1);
        }
        return _head[pos];
    }

    string::string(const char *buffer)
    {
        int size = strlen(buffer);
        expand(size);
        strncpy(_head, buffer, sizeof(buffer));
        _size = size;
    }

    string::string(const mycode::string &buffer)
    {
        mycode::string tmp(buffer.c_str());
        _head = tmp._head;
        _size = tmp._size;
        _capacity = tmp._capacity;
    }

    string::string(mycode::string &&buffer)
    {
        std::swap(_head, buffer._head);
        std::swap(_size, buffer._size);
        std::swap(_capacity, buffer._capacity);
    }

    string &string::operator+=(const mycode::string &str)
    {
        if (_capacity < _size + str.size())
            expand(_capacity + str.size());
        strncpy(_head + _size, str._head, str.size());
        return *this;
    }

    string &string::operator+=(const char *buffer)
    {
        this->operator+=({buffer});
        return *this;
    }

    void string::append(const string &str)
    {
        this->operator+=(str);
    }

    void string::append(char ch)
    {
        push_back(ch);
    }

    void string::push_back(char ch)
    {
        expand(_size + 1);
        _head[_size++] = ch;
    }

    void string::pop_back()
    {
        if(size() == 0) std::cerr << "this obj is empty" << std::endl;
        _head[--_size] = 0;
    }

    void string::clear()
    {
        release();
    }

    void string::swap(string& buffer)
    {
        std::swap(_head, buffer._head);
        std::swap(_size, buffer._size);
        std::swap(_capacity, buffer._capacity);
    }

    void string::copy(const string & buffer)
    {
        mycode::string tmp(buffer);
        this->swap(tmp);
    }

    bool string::compare(const string &buffer)
    {
        return strcmp(_head,buffer._head);
    }
    size_t string::find(const string& str) const
    {
        // 这里使用kmp算法
        // 得到next数组
        
    }

    size_t string::find(char ch) const 
    {
        for(size_t i = 0; i < _size; i++)
            if(_head[i] == ch) return i;
        return NPOS;// 说明没有找到
    } 
}
