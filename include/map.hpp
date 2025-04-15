//
// Created by jerry on 2024/7/31.
//

#ifndef MAP_H
#define MAP_H

#include "RBTree.hpp"
// map is based on the RBTree
// but the main question is that we can't find a fit constractor to make iterator to fit const_iterator

namespace mycode
{
    template <class K, class V, class Compare>
    class map
    {
    public:
        explicit map() = default;
        typedef pair<K, V> PairKV;
        struct MapKOfT
        {
            const K &operator()(const PairKV &kv) { return kv.first; }
        };

        typedef RBTree<K, pair<K, V>, MapKOfT, Compare> _rb_Tree;
        // typedef typename _rb_Tree::iterator iterator;
        // typedef typename _rb_Tree::iterator const_iterator;
        using iterator = typename RBTree<K,std::pair<K,V>,MapKOfT>::iterator;
        using const_iterator = typename RBTree<K,std::pair<K,V>,MapKOfT>::const_iterator;
        bool erase(const K &key) { return _rbt.erase(key); }
        bool insert(const pair<K, V> &kv)
        {
            return _rbt.insert(kv);
        }
        bool insert(std::pair<K, V> && kv){ return _rbt.insert(std::move(kv)); };
        bool insert(const std::initializer_list<pair<K, V>> &il)
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
        mycode::RBTree<K, std::pair<K, V>, MapKOfT, Compare> _rbt;
    };

};

namespace StandardModule
{
    template <class T,class V,class Compare = std::less<T>>
    using map = mycode::map<T,V,Compare>;
}
#endif // MAP_H
