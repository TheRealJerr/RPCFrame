#include <iostream>
#ifndef __SHARED_PTR__
#define __SHARED_PTR__
#include "unique_ptr.hpp"
namespace mycode
{
    template <class PtrType, class Del = Deltor<PtrType>>
    class shared_ptr
    {
    public:
        shared_ptr(PtrType *ptr) : _ptr(ptr), _pcount(new int(1))
        {
        }
        using Ref = PtrType &;
        using Ptr = PtrType *;
        using self = shared_ptr<PtrType, Del>;

        Ref operator*() { return *_ptr; }
        Ptr operator->() { return _ptr; }
        operator PtrType *() { return _ptr; }

        ~shared_ptr()
        {
            if ((*_pcount) > 1)
                (*_pcount)--;
            else
            {
                _deltor(_ptr);
                delete _pcount;
                _pcount = nullptr;
            }
        }

        // 禁止拷贝构造和拷贝赋值
        shared_ptr(const self &other) : _ptr(other._ptr)
        {
            (*_pcount)++;
        }
        self &operator=(const self &other)
        {
            if (this == &other)
                return *this;

            this->~shared_ptr(); // 将自身本身进行析构
            _ptr = other._ptr;
            (*_pcount)++;
            return *this;
        }

        // 允许移动构造和移动赋值
        shared_ptr(self &&other) noexcept : _ptr(other._ptr)
        {
            (*_pcount)++;
        }

        self &operator=(self &&other) noexcept
        {
            if (this == &other)
                return *this;
            this->~shared_ptr(); // 将自身本身进行析构
            _ptr = other._ptr;
            (*_pcount)++;
            return *this;
        }

    private:
        Ptr _ptr;
        int *_pcount = nullptr;
        Del _deltor; // 用于释放指针
    };
    template <class PtrType, class Del>
    class shared_ptr<PtrType[],Del>
    {
    public:
        shared_ptr(PtrType *ptr) : _ptr(ptr), _pcount(new int(1))
        {
        }
        using Ref = PtrType &;
        using Ptr = PtrType *;
        using self = shared_ptr<PtrType, Del>;

        Ref operator*() { return *_ptr; }
        Ptr operator->() { return _ptr; }
        operator PtrType *() { return _ptr; }

        ~shared_ptr()
        {
            if ((*_pcount) > 1)
                (*_pcount)--;
            else
            {
                _deltor(_ptr);
                delete _pcount;
                _pcount = nullptr;
            }
        }

        // 禁止拷贝构造和拷贝赋值
        shared_ptr(const self &other) : _ptr(other._ptr)
        {
            (*_pcount)++;
        }
        self &operator=(const self &other)
        {
            if (this == &other)
                return *this;

            this->~shared_ptr(); // 将自身本身进行析构
            _ptr = other._ptr;
            (*_pcount)++;
            return *this;
        }

        // 允许移动构造和移动赋值
        shared_ptr(self &&other) noexcept : _ptr(other._ptr)
        {
            (*_pcount)++;
        }

        self &operator=(self &&other) noexcept
        {
            if (this == &other)
                return *this;
            this->~shared_ptr(); // 将自身本身进行析构
            _ptr = other._ptr;
            (*_pcount)++;
            return *this;
        }

    private:
        Ptr _ptr;
        int *_pcount = nullptr;
        Del _deltor; // 用于释放指针
    };
    // template <class PtrType, class Del>
    // class unique_ptr<PtrType[], Del>
    // {
    // public:
    //     unique_ptr(PtrType *ptr) : _ptr(ptr) {}

    //     using Ref = PtrType &;
    //     using Ptr = PtrType *;
    //     using self = unique_ptr<PtrType[], Del>;

    //     Ref operator*() { return *_ptr; }
    //     Ptr operator->() { return _ptr; }
    //     operator PtrType *() { return _ptr; }

    //     ~unique_ptr()
    //     {
    //         _deltor(_ptr);
    //     }

    //     // 禁止拷贝构造和拷贝赋值
    //     unique_ptr(const self &) = delete;
    //     unique_ptr &operator=(const self &) = delete;

    //     // 允许移动构造和移动赋值
    //     unique_ptr(self &&other) noexcept : _ptr(other._ptr)
    //     {
    //         other._ptr = nullptr;
    //     }

    //     self &operator=(self &&other) noexcept
    //     {
    //         if (this != &other)
    //         {
    //             _deltor(_ptr);
    //             _ptr = other._ptr;
    //             other._ptr = nullptr;
    //         }
    //         return *this;
    //     }

    // private:
    //     PtrType *_ptr;
    //     Del _deltor; // 用于释放指针
    // };

}
namespace StandardModule
{
    template <class T,class Del>
    using shared_ptr = mycode::shared_ptr<T,Del>;
}
#endif