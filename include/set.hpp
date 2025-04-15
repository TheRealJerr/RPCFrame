#ifndef __SET__
#define __SET__
#include "RBTree.hpp"
namespace mycode
{
    template <class K, class Compare>
    class set
    {
    public:
        explicit set() = default;

        struct SetKOfT
        {
            const K &operator()(const K &v) { return v; }
        };

        typedef RBTree<K, K, SetKOfT, Compare> _rb_Tree;
        // typedef typename _rb_Tree::iterator iterator;
        // typedef typename _rb_Tree::iterator const_iterator;
        using iterator = typename RBTree<K, K, SetKOfT>::iterator;
        using const_iterator = typename RBTree<const K, const K, SetKOfT>::const_iterator;
        bool erase(const K &key) { return _rbt.erase(key); }
        bool insert(const K &val)
        {
            return _rbt.insert(val);
        }
        bool insert(K &&val) { return _rbt.insert(std::move(val)); };
        bool insert(const std::initializer_list<K> &il)
        {
            for (auto &e : il)
                insert(e);
            return true;
        }
        std::size_t size() const { return _rbt.size(); }
        iterator begin() { return _rbt.begin(); }
        iterator end() { return _rbt.end(); }
        const_iterator begin() const { return _rbt.begin(); }
        const_iterator end() const { return _rbt.end(); }

    private:
        mycode::RBTree<K, K, SetKOfT, Compare> _rbt;
    };
}
namespace StandardModule
{
    template <class T,class Compare = std::less<T>>
    using Set = mycode::set<T,Compare>;
}
#endif