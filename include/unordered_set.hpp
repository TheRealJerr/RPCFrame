//
// Created by jerry on 2024/8/12.
//

#ifndef UNORDERED_SET_H
#define UNORDERED_SET_H
#include "Harsh.hpp"

using std::forward;

namespace mycode {
    template <class K,class harsh = std::hash<K>>
    class unordered_set {
    public:
        struct SetKeyOfT {
            const K& operator()(const K& key) const { return key; }
        };
        using iterator = typename Harsh_bucket::HarshTable<const K,const K,SetKeyOfT,harsh>::iterator;
        using const_iterator = typename Harsh_bucket::HarshTable<const K,const K,SetKeyOfT,harsh>::const_iterator;
        template <class Elem>
        std::pair<iterator,bool> insert(Elem&& key) { return _hb.insert(std::forward<Elem>(key)); }

        bool erase(const K& key) { return _hb.erase(key); }
        iterator find(const K& key) { return _hb.find(key); }
        iterator begin() { return _hb.begin(); }
        iterator end() { return _hb.end(); }
        const_iterator begin() const { return _hb.begin(); }
        const_iterator end() const { return _hb.end(); }

    private:
        Harsh_bucket::HarshTable<const K,const K,SetKeyOfT,harsh> _hb;
    };
}
namespace StandardModule
{
    template <class K,class Harsh = std::hash<K>>
    using unordered_set = mycode::unordered_set<K,Harsh>;
}
#endif //UNORDERED_SET_H
