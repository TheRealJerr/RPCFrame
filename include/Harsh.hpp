//
// Created by jerry on 2024/8/3.
//

#ifndef HARSH_H
#define HARSH_H
#include <iostream>
using std::hash;
#include <vector>
using std::vector;
using std::pair;
namespace Harsh_bucket {//拉链法实现哈希桶

    template <class V>
    struct HarshDataType {
        template <class Elem>
        HarshDataType(Elem&& kv):_kv(std::forward<Elem>(kv))
        {}
        V _kv;
        HarshDataType<V>* _next =  nullptr;
    };
    template <class K,class V,class KOfT,class Harsh>
    class HarshTable;
    template <class K,class V,class KOfT,class Harsh>
    struct HarshIterator {
        typedef HarshDataType<V> Node;
        typedef HarshIterator<K,V,KOfT,Harsh>& Self;
        HarshIterator(HarshTable<K,V,KOfT,Harsh>* tablePtr,Node* node,size_t harsh):_tablePtr(tablePtr),_node(node),_harsh(harsh){}


        Self operator++() {
            if(_node->_next) {
                _node = _node->_next;
                return *this;
            }
            _harsh++;
            while(_tablePtr->_table[_harsh] == nullptr && _harsh < (*_tablePtr)._table.size()) {
                _harsh++;
            }
            if(_harsh == (*_tablePtr)._table.size()) _node = nullptr;
            else _node = (*_tablePtr)._table[_harsh];
            return *this;
        }
        bool operator!=(const HarshIterator<K,V,KOfT,Harsh>& it){ return _node != it._node; }
        V& operator*() { return _node->_kv; }
        V* operator->() { return &(_node->_kv); }

        HarshTable<K,V,KOfT,Harsh>* _tablePtr;
        Node* _node;//存储当前的节点
        size_t _harsh = 0;
    };
    template <class K,class V,class KOfT,class Harsh>
    struct HarshConstIterator {
        typedef HarshDataType<V> Node;
        typedef const HarshConstIterator<K,V,KOfT,Harsh>& Self;
        HarshConstIterator(HarshTable<K,V,KOfT,Harsh>* tablePtr,Node* node,size_t harsh):_tablePtr(tablePtr),_node(node),_harsh(harsh){}
        explicit HarshConstIterator(HarshIterator<K,V,KOfT,Harsh>&& it) {
            _node = it._node;
            _tablePtr = it._tablePtr;
            _harsh = it._harsh;
        }
        explicit HarshConstIterator(HarshConstIterator<K,V,KOfT,Harsh>&& it)//拷贝构造
        {
            _node = it._node;
            _tablePtr = it._tablePtr;
            _harsh = it._harsh;
        }
        const HarshTable<K,V,KOfT,Harsh>* _tablePtr;
        const Node* _node;//存储当前的节点
        size_t _harsh = 0;
        Self operator++() {
            if(_node->_next) {
                _node = _node->_next;
                return *this;
            }
            _harsh++;
            while(_tablePtr->_table[_harsh] == nullptr && _harsh < (*_tablePtr)._table.size()) {
                _harsh++;
            }
            if(_harsh == (*_tablePtr)._table.size()) _node = nullptr;
            else _node = (*_tablePtr)._table[_harsh];
            return *this;
        }
        bool operator!=(const HarshConstIterator<K,V,KOfT,Harsh>& it) const { return _node != it._node; }
        const V& operator*() { return _node->_kv; }
        const V* operator->() { return &(_node->_kv); }
    };
    template <class K,class V,class KOfT,class Harsh>
    bool operator==(const HarshConstIterator<K,V,KOfT,Harsh>& lhs,const HarshConstIterator<K,V,KOfT,Harsh>& rhs) {
        return lhs._node == rhs._node;
    }
    template <class K,class V,class KOfT,class Harsh>
    class HarshTable {
    public:
        HarshTable(HarshTable&& hb) = default;//这里在底层调用vector的右值赋值重载
        friend struct HarshIterator<K,V,KOfT,Harsh>;
        friend struct HarshConstIterator<K,V,KOfT,Harsh>;
        typedef HarshDataType<V>* Ptr;
        typedef HarshDataType<V> Node;
        typedef HarshIterator<K,V,KOfT,Harsh> iterator;
        typedef HarshConstIterator<K,V,KOfT,Harsh> const_iterator;
        HarshTable(){ _table.resize(10); }
        void insertHelper() {
            HarshTable tmp;
            tmp._table.resize(_table.size() * 2);
            for(auto& e : _table) {
                Ptr cur = e;
                while(cur) {
                    Ptr next = cur->_next;
                    size_t harsh = _harsh(_koft(std::forward<V>(cur->_kv))) % tmp._table.size();
                    if(tmp._table[harsh] == nullptr) {
                        tmp._table[harsh] = cur;
                        cur->_next = nullptr;
                    }
                    else {
                        cur->_next = tmp._table[harsh];
                        tmp._table[harsh] = cur;
                    }
                    cur = next;
                 }
                 e = nullptr;
            }
            _table.swap(tmp._table);

        }
        template <class Elem>
        pair<iterator,bool> insert(Elem&& kv) {

            iterator it = find(_koft(kv));
            if(it != end()) return make_pair(it,false);
            int harsh = _harsh(_koft(kv)) % _table.size();
            Ptr newnode = new HarshDataType<V>(std::forward<Elem>(kv));
            if(_table[harsh] == nullptr) _table[harsh] = newnode;
            else {
                newnode->_next = _table[harsh];
                _table[harsh] = newnode;
            }
            _size++;
            if((_size * 10) / _table.size() >= 10) insertHelper();
            return std::make_pair(iterator(this,newnode,_harsh(_koft(kv)) % _table.size()),true);
        }
        ~HarshTable() {
            for(auto& e : _table) {
                Ptr cur = e;
                while(cur) {
                    Ptr next = cur->_next;
                    delete cur;
                    cur = next;
                }
               e = nullptr;
            }
        }
        iterator find(const K& key) {
            size_t harsh = _harsh(key) % _table.size();
            Ptr cur = _table[harsh];
            while(cur) {
                if(_koft(std::forward<V>(cur->_kv)) == key) return iterator(this,cur,harsh);
                cur = cur->_next;
            }
            return iterator(this,nullptr,_table.size());
        }
        bool erase(const K& key){
            if(find(key) == nullptr) return false;
            size_t harsh = _harsh(key) % _table.size();
            Ptr cur = _table[harsh];
            Ptr prev = nullptr;
            while(cur->_kv.first != key) {
                prev = cur;
                cur = cur->_next;
            }
            if(prev == nullptr) _table[harsh] = cur->_next;
            else prev->_next = cur->_next;
            delete cur;
            _size--;
            return true;
        }
        iterator begin() {
            typename vector<HarshDataType<V>*>::iterator it = _table.begin();
            while(it != _table.end()) {
                if((*it) != nullptr) return iterator(this,(*it),it - _table.begin());
                it++;
            }
            return iterator(this,nullptr,_table.size());
        }
        iterator end() {
            return iterator(this,nullptr,_table.size());
        }
        const_iterator begin() const {
            typename vector<HarshDataType<V>*>::const_iterator it = _table.begin();
            while(it != _table.end()) {
                if((*it) != nullptr) return const_iterator(this,(*it),it - _table.begin());
                it++;
            }
            return const_iterator(this,nullptr,_table.size());
        }
        const_iterator end() const { return const_iterator(nullptr);}

    private:
        vector<HarshDataType<V>*> _table;
        size_t _size = 0;
        Harsh _harsh;
        KOfT _koft;
    };
}
#endif //HARSH_H
