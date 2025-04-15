#ifndef __BIT_MAP__
#define __BIT_MAP
#include <iostream>
#include <cassert>
namespace mycode{
    class bitmap{
    public:
        bitmap(int size)
        {
            assert(size);
            _bitmap_size = size % 8 == 0 ? size / 8 : (size / 8 + 1);
            _arr = new size_t[_bitmap_size]{0};

        }
        ~bitmap(){
            delete _arr;
            _count = _bitmap_size = 0;
        }
        
        void set(int pos)
        {
            assert(pos > 0);
            _arr[pos / 8] |= (1<<(pos % 8));
            _count++;
        }
        void reset(int pos)
        {
            assert(pos > 0);
            _arr[pos / 8] &= (~(1<<(pos%8)));
        }
        size_t count(){
            return _count;   
        }
        bool test(int pos)
        {
            return (_arr[pos / 8] << (pos % 8) & 1);
        }
    private:
        size_t* _arr = nullptr;
        int _bitmap_size = 0;
        int _count = 0;
    };
}
namespace StandardModule
{
    using bitmap = mycode::bitmap;
}
#endif