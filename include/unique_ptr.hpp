#ifndef __UNIQUE_PTR__
#define __UNIQUE_PTR__
#include <iostream>
namespace mycode
{
    template <class PtrType>
    struct Deltor
    {
        void operator()(PtrType *ptr)
        {
            delete ptr;
        }
    };

    template <class PtrType>
    struct Deltor<PtrType[]>
    {
        void operator()(PtrType *ptr)
        {
            delete[] ptr;

        }
    };
    template <class PtrType, class Del = Deltor<PtrType>>
    class unique_ptr
    {
    public:
        unique_ptr(PtrType *ptr) : _ptr(ptr) {}

        using Ref = PtrType &;
        using Ptr = PtrType *;
        using self = unique_ptr<PtrType, Del>;

        Ref operator*() { return *_ptr; }
        Ptr operator->() { return _ptr; }
        operator PtrType *() { return _ptr; }

        ~unique_ptr()
        {
            _deltor(_ptr);
        }

        // 禁止拷贝构造和拷贝赋值
        unique_ptr(const self &) = delete;
        unique_ptr &operator=(const self &) = delete;

        // 允许移动构造和移动赋值
        unique_ptr(self &&other) noexcept : _ptr(other._ptr)
        {
            other._ptr = nullptr;
        }

        self &operator=(self &&other) noexcept
        {
            if (this != &other)
            {
                _deltor(_ptr);
                _ptr = other._ptr;
                other._ptr = nullptr;
            }
            return *this;
        }

    private:
        PtrType *_ptr;
        Del _deltor; // 用于释放指针
    };
    template <class PtrType, class Del>
    class unique_ptr<PtrType[], Del>
    {
    public:
        unique_ptr(PtrType *ptr) : _ptr(ptr) {}

        using Ref = PtrType &;
        using Ptr = PtrType *;
        using self = unique_ptr<PtrType[], Del>;

        Ref operator*() { return *_ptr; }
        Ptr operator->() { return _ptr; }
        operator PtrType *() { return _ptr; }

        ~unique_ptr()
        {
            _deltor(_ptr);
        }

        // 禁止拷贝构造和拷贝赋值
        unique_ptr(const self &) = delete;
        unique_ptr &operator=(const self &) = delete;

        // 允许移动构造和移动赋值
        unique_ptr(self &&other) noexcept : _ptr(other._ptr)
        {
            other._ptr = nullptr;
        }

        self &operator=(self &&other) noexcept
        {
            if (this != &other)
            {
                _deltor(_ptr);
                _ptr = other._ptr;
                other._ptr = nullptr;
            }
            return *this;
        }

    private:
        PtrType *_ptr;
        Del _deltor; // 用于释放指针
    };

}
#endif