//
// Created by jerry on 2024/8/6.
//

#ifndef UNORDERED_MAP_H
#define UNORDERED_MAP_H

#include <iostream>
#include "Harsh.hpp"
using std::size_t;
using std::pair;
namespace mycode {
    template <class K>
    struct HarshTrans {
        size_t operator()(const K& key) const {
           return (size_t)key;
        }
    };

    template <class K,class V,class harsh>
    class unordered_map {
    public:
        struct MapKeyOfT
        {
            const K& operator()(const std::pair<K,V>& kv){
                return kv.first;
            }
        };

        typedef typename Harsh_bucket::HarshTable<K,pair<const K,V>,MapKeyOfT,harsh>::iterator iterator;
        typedef typename Harsh_bucket::HarshTable<K,pair<const K,V>,MapKeyOfT,harsh>::const_iterator const_iterator;
        pair<iterator,bool> insert(const pair<K,V>& kv) { return _hb.insert(kv); }
        pair<iterator,bool> insert(pair<K,V>&& kv) { return _hb.insert(std::forward<pair<K,V>>(kv)); }//这里由于用了右值引用得加上forwar
        bool erase(const K& key) { return _hb.erase(key); }
        iterator find(const K& key) { return _hb.find(key); }
        iterator begin() { return _hb.begin(); }
        iterator end() { return _hb.end(); }
        const_iterator begin() const { return _hb.begin(); }
        const_iterator end() const { return _hb.end(); }
        V& operator[](K&& key) {
            pair<iterator,bool> ret = _hb.insert(make_pair(key,V()));
            return ret.first->second;
        }

    private:
        Harsh_bucket::HarshTable<K,pair<const K,V>,MapKeyOfT,harsh> _hb;
    };
}
#include <iostream>
namespace StandardModule
{
    template <class K,class V,class Harsh = std::hash<K>>
    using unordered_map = mycode::unordered_map<K,V,Harsh>;
}


#endif //UNORDERED_MAP_H
