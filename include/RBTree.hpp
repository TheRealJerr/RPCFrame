//
// Created by jerry on 2024/7/30.
//
//自建红黑树
#include <vector>
using std::vector;
#ifndef RBTREE_H
#define RBTREE_H
#include <iostream>
using std::size_t;
using std::pair;
using std::cin;
using std::cout;
using std::endl;


namespace mycode {
    enum Color {
        Red,
        Black,
    };
    template <class data>
    struct RBTreeNode {
        RBTreeNode(const data& kv,RBTreeNode<data>* left = nullptr\
            ,RBTreeNode<data>* right = nullptr,RBTreeNode<data>* parent = nullptr):\
        _kv(kv),_left(left),_right(right),_parent(parent)
        {}
        data _kv;
        RBTreeNode<data>* _left;
        RBTreeNode<data>* _right;
        RBTreeNode<data>* _parent;
        Color _col;
    };
    template <class data,class Node,class Ptr>
    struct RBTreeIterator {
        typedef RBTreeIterator<data,Node,Ptr> Self;
        Ptr _cur;
        explicit RBTreeIterator(Node* cur):_cur(cur){}
        // explicit RBTreeIterator(const RBTreeIterator<data,Node,Ptr>& rbt) { _cur = rbt._cur; }
        bool operator!=(const Self& it) { return _cur != it._cur; }
        Self& operator=(const Self& it) { _cur = it._cur; return *this; }
        data& operator*() { return _cur->_kv; }
        data* operator->() { return &(_cur->_kv); }
        Self& operator++() {
            Node* cur = _cur;
            if(cur->_right) {
                cur = cur->_right;
                while(cur->_left) { cur = cur->_left; }
                _cur = cur;
            }else {
                while(cur) {
                    if(cur->_parent && cur == cur->_parent->_left) {
                        _cur = cur->_parent;
                        return *this;
                    }else cur = cur->_parent;
                }
            }
            _cur = cur;
            return *this;
        }
    };
    template <class K,class V,class KOfT,class Compare = std::less<K>>
    class RBTree {
    public:

        typedef RBTreeNode<V> Node;
        typedef RBTreeIterator<const V,Node,Node*> iterator;
        typedef RBTreeIterator<const V,Node,Node*> const_iterator;
        size_t size() const { return _size; }
        void RotateLeft(Node* parent);
        void RotateRight(Node* parent);
        bool insert(const V& kv);
        bool insert(V&& kv);
        bool erase(const K& key);
        // bool erase(const K& val);
        Node* getRoot() { return _root; }
        iterator begin() {
            Node* cur = _root;
            if(cur == nullptr) return iterator(nullptr);
            while(cur->_left) {
                cur = cur->_left;
            }
            return iterator(cur);
        }
        iterator end() { return iterator(nullptr);}
        // debug模块
        // pair<bool,int> isTheRightRBTree(Node* root) {
        //     if(root == nullptr) return std::make_pair(true,1);
        //     pair<bool,int> lhs = isTheRightRBTree(root->_left);
        //     if(lhs.first == false) return std::make_pair(false,-1);
        //     pair<bool,int> rhs = isTheRightRBTree(root->_right);
        //     if(rhs.first == false) return std::make_pair(false,-1);
        //     if(lhs.second != rhs.second) return std::make_pair(false,-1);
        //     if(root->_col == Red) {
        //         if((root->_left && root->_left->_col == Red) || (root->_right && root->_right->_col == Red))
        //             return {false,-1};
        //         return { true,rhs.second};
        //     }else return { true,rhs.second + 1};
        // }
        const_iterator begin() const {
            Node* cur = _root;
            if(cur == nullptr) return const_iterator(nullptr);
            while(cur->_left) {
                cur = cur->_left;
            }
            return const_iterator(cur);
        }
        bool insert(const std::initializer_list<V>& il) {
            for(auto& e: il) {
                this->insert(e);
            }
            return true;
        }
        const_iterator end() const { return const_iterator(nullptr); }
        // debug模块
        // void inorderHelper(Node* root,vector<Node*>& sk) {
        //     if(root == nullptr) {
        //         int blackSize = 0;
        //         for(auto& e : sk) {
        //             std::cout << e->_kv.first << " ";
        //             if(e->_col == Black) {
        //                 std::cout << "Black ";
        //                 blackSize++;
        //             }
        //             else std::cout << "Red ";
        //         }
        //         std::cout << "nullptr " << blackSize << std::endl;
        //         return;
        //     }
        //     if(root->_left) sk.push_back(root->_left);
        //     inorderHelper(root->_left,sk);
        //     if(root->_left) sk.pop_back();
        //     if(root->_right) sk.push_back(root->_right);
        //     inorderHelper(root->_right,sk);
        //     if(root->_right) sk.pop_back();
        // }
        // void inorder() {
        //     vector<Node*> sk;
        //     sk.push_back(_root);
        //     inorderHelper(_root,sk);
        // }
    private:
        Node* _root = nullptr;
        std::size_t _size = 0;
        Compare _cmp;
        KOfT _koft;
    };
    template <class K,class V,class KOfT,class Compare>
    bool RBTree<K,V,KOfT,Compare>::insert(const V& kv) {

        enum RBDirct {
            left,
            right,
        };
        enum RBDirct rb_dirct = left;
        Node* parent = nullptr;
        Node* cur = _root;
        if(cur == nullptr) {
            _root = new RBTreeNode<V>(kv);
            _size++;
            return true;
        }
        while(cur) {
            parent = cur;
            if(_cmp(_koft(kv),_koft(cur->_kv))) {
                cur = cur->_left;
                rb_dirct = left;
            }
            else if(_cmp(_koft(cur->_kv),_koft(kv))) {
                cur = cur->_right;
                rb_dirct = right;
            }
            else return false;
        }

        cur = new Node(kv);
        _size++;
        cur->_col = Red;//新创建节点如果不是根节点,直接给红色的节点
        //cur这里是新增的节点
        cur->_parent = parent;
        if(rb_dirct == left) parent->_left = cur;
        else parent->_right = cur;
        while(cur) {
            parent = cur->_parent;
            if(parent == nullptr) {
                cur->_col = Black;
                break;
            }else if(parent->_col == Black) break;
            Node* grandParent = parent->_parent;
            if(grandParent == nullptr) {
                parent->_col = Black;
                break;
            }else {
                Node* uncle = nullptr;
                if(parent == grandParent->_left) uncle = grandParent->_right;
                else uncle = grandParent->_left;
                if(uncle && uncle->_col == Red) {
                    parent->_col = uncle->_col = Black;
                    grandParent->_col = Red;
                    cur = grandParent;
                }else {//叔叔不存在 / 叔叔存在且为黑
                    if(uncle == nullptr) {
                        if(uncle == grandParent->_left) {
                            if(cur == parent->_right) {
                                RotateLeft(grandParent);
                                grandParent->_col = Red;
                                parent->_col = Black;
                            }
                            else {
                                RotateRight(parent);
                                RotateLeft(grandParent);
                                cur->_col = Black;
                                grandParent->_col = Red;
                            }

                        }
                        else {
                            if(cur == parent->_left) {
                                RotateRight(grandParent);
                                grandParent->_col = Red;
                                parent->_col = Black;
                            }
                            else {
                                RotateLeft(parent);
                                RotateRight(grandParent);
                                cur->_col = Black;
                                grandParent->_col = Red;
                            }
                        }

                    }else {
                        if(uncle == grandParent->_left) {
                            if(cur == parent->_right) {
                                RotateLeft(grandParent);
                                grandParent->_col = Red;
                                parent->_col = Black;
                            }
                            else {
                                RotateRight(parent);
                                RotateLeft(grandParent);
                                cur->_col = Black;
                                grandParent->_col = parent->_col = Red;
                            }
                        }else {
                            if(cur == parent->_left) {
                                RotateRight(grandParent);
                                grandParent->_col = Red;
                                parent->_col = Black;
                            }else {
                                RotateLeft(parent);
                                RotateRight(grandParent);
                                cur->_col = Black;
                                grandParent->_col = parent->_col = Red;
                            }
                        }
                    }
                    break;
                }
            }
        }

        // while(cur) {
        //     if(cur->_parent == nullptr) break;
        //     else if(cur->_parent->_col == Red) {
        //         parent = cur->_parent;
        //         if(parent->_parent == nullptr) parent->_col = Black;//这里说明parent是root
        //         else {
        //             Node* grandParent = parent->_parent;
        //             Node* uncle = nullptr;
        //             if(parent == grandParent->_left) uncle = grandParent->_right;
        //             else uncle = grandParent->_right;
        //             if(uncle == nullptr) {
        //                 if(uncle == grandParent->_left) RotateLeft(grandParent);
        //                 else RotateRight(grandParent);
        //                 parent->_col = Black;
        //                 grandParent->_col = Red;
        //                 cur = parent;
        //                 if(parent->_parent == nullptr) _root = parent;
        //                 break;
        //             }else if(uncle->_col == Red) {
        //                 parent->_col = uncle->_col = Black;
        //                 grandParent->_col = Red;
        //                 cur = grandParent;
        //             }else {
        //
        //             }
        //         }
        //     }else break;
        // }
        return true;
    }
    template <class K,class V,class KOfT,class Compare>
    bool RBTree<K,V,KOfT,Compare>::insert(V&& kv) {

        enum RBDirct {
            left,
            right,
        };
        enum RBDirct rb_dirct = left;
        Node* parent = nullptr;
        Node* cur = _root;
        if(cur == nullptr) {
            _root = new RBTreeNode<V>(std::forward<V>(kv));
            _size++;
            return true;
        }
        while(cur) {
            parent = cur;
            if(_cmp(_koft(kv),_koft(cur->_kv))) {
                cur = cur->_left;
                rb_dirct = left;
            }
            else if(_cmp(_koft(cur->_kv),_koft(kv))) {
                cur = cur->_right;
                rb_dirct = right;
            }
            else return false;
        }

        cur = new Node(kv);
        _size++;
        cur->_col = Red;//新创建节点如果不是根节点,直接给红色的节点
        //cur这里是新增的节点
        cur->_parent = parent;
        if(rb_dirct == left) parent->_left = cur;
        else parent->_right = cur;
        while(cur) {
            parent = cur->_parent;
            if(parent == nullptr) {
                cur->_col = Black;
                break;
            }else if(parent->_col == Black) break;
            Node* grandParent = parent->_parent;
            if(grandParent == nullptr) {
                parent->_col = Black;
                break;
            }else {
                Node* uncle = nullptr;
                if(parent == grandParent->_left) uncle = grandParent->_right;
                else uncle = grandParent->_left;
                if(uncle && uncle->_col == Red) {
                    parent->_col = uncle->_col = Black;
                    grandParent->_col = Red;
                    cur = grandParent;
                }else {//叔叔不存在 / 叔叔存在且为黑
                    if(uncle == nullptr) {
                        if(uncle == grandParent->_left) {
                            if(cur == parent->_right) {
                                RotateLeft(grandParent);
                                grandParent->_col = Red;
                                parent->_col = Black;
                            }
                            else {
                                RotateRight(parent);
                                RotateLeft(grandParent);
                                cur->_col = Black;
                                grandParent->_col = Red;
                            }

                        }
                        else {
                            if(cur == parent->_left) {
                                RotateRight(grandParent);
                                grandParent->_col = Red;
                                parent->_col = Black;
                            }
                            else {
                                RotateLeft(parent);
                                RotateRight(grandParent);
                                cur->_col = Black;
                                grandParent->_col = Red;
                            }
                        }

                    }else {
                        if(uncle == grandParent->_left) {
                            if(cur == parent->_right) {
                                RotateLeft(grandParent);
                                grandParent->_col = Red;
                                parent->_col = Black;
                            }
                            else {
                                RotateRight(parent);
                                RotateLeft(grandParent);
                                cur->_col = Black;
                                grandParent->_col = parent->_col = Red;
                            }
                        }else {
                            if(cur == parent->_left) {
                                RotateRight(grandParent);
                                grandParent->_col = Red;
                                parent->_col = Black;
                            }else {
                                RotateLeft(parent);
                                RotateRight(grandParent);
                                cur->_col = Black;
                                grandParent->_col = parent->_col = Red;
                            }
                        }
                    }
                    break;
                }
            }
        }

        // while(cur) {
        //     if(cur->_parent == nullptr) break;
        //     else if(cur->_parent->_col == Red) {
        //         parent = cur->_parent;
        //         if(parent->_parent == nullptr) parent->_col = Black;//这里说明parent是root
        //         else {
        //             Node* grandParent = parent->_parent;
        //             Node* uncle = nullptr;
        //             if(parent == grandParent->_left) uncle = grandParent->_right;
        //             else uncle = grandParent->_right;
        //             if(uncle == nullptr) {
        //                 if(uncle == grandParent->_left) RotateLeft(grandParent);
        //                 else RotateRight(grandParent);
        //                 parent->_col = Black;
        //                 grandParent->_col = Red;
        //                 cur = parent;
        //                 if(parent->_parent == nullptr) _root = parent;
        //                 break;
        //             }else if(uncle->_col == Red) {
        //                 parent->_col = uncle->_col = Black;
        //                 grandParent->_col = Red;
        //                 cur = grandParent;
        //             }else {
        //
        //             }
        //         }
        //     }else break;
        // }
        return true;
    }
    template <class K,class V,class KOfT,class Compare>
    bool RBTree<K,V,KOfT,Compare>::erase(const K& key){
        mycode::Color colDel = Red;
        Node* cur = _root;
        while(true){
            if(cur == nullptr) return false;
            else if(_cmp(key,_koft(cur->_kv))) cur = cur->_left;
            else if(_cmp(_koft(cur->_kv),key)) cur = cur->_right;
            else break;
        }
        //这里说明cur就是我们想要删除的节点
        //如果有两个孩子节点,那么我们就对其进行找到后继节点的操作
        if(cur->_left && cur->_right){
            Node* rightMin = cur->_right;
            while(rightMin->_left) rightMin = rightMin->_left;
            //这个时候就是找到了后继的节点
            std::swap(cur->_kv,rightMin->_kv);
            cur = rightMin;
        }
        colDel = cur->_col;//这个记录的删除节点的额颜色
        Node* parent = cur->_parent;
        Node* child = nullptr;
        if(cur->_left == nullptr) child = cur->_right;
        else child = cur->_left;
        if(parent) {
            if(cur == parent->_left) parent->_left = child;
            else parent->_right = child;
        }else child = _root;//这种情况说明我们删除的节点是根节点
        if(child) child->_parent = parent;
        delete cur;
        cur = child;
        //ok删除结束了这个时候我们需要对红黑树进行重构
        if(colDel == Red) return true;//这里就是太棒了,删除红节点,直接结束
        while(parent){
            Node* bro = nullptr;
            if(cur == parent->_left) bro = parent->_right;
            else bro = parent->_left;
            if(bro->_col == Red){
                if(bro == parent->_right) RotateLeft(parent);
                else RotateRight(parent);
                parent->_col = Red;
                bro->_col = Black;
            }else{//bro->_col == Black
                if((bro->_left == nullptr && bro->_right == nullptr) || (bro->_left && bro->_left->_col == Black && bro->_right && bro->_right->_col == Black)){
                   if(parent->_col == Red){
                       parent->_col = Black;
                       bro->_col = Red;
                       break;
                   }else {
                       bro->_col = Red;
                       cur = parent;
                       parent = parent->_col;
                   }
                }
                if(bro == parent->_right && bro->_right && bro->_right->_col == Red){
                    RotateLeft(parent);
                    bro->_col = parent->_col;
                    parent->_col = bro->_right->_col = Black;
                }else if(bro == parent->_right && bro->_left && bro->_left->_col == Red){
                    RotateRight(bro);
                    RotateLeft(parent);
                    bro->_parent->_col = bro->_parent->_left->_col;
                    parent->_col = bro->_col = Black;
                }else if(bro == parent->_left && bro->_left && bro->_left->_col = Red){
                    RotateRight(parent);
                    bro->_col = parent->_col;
                    parent->_col = bro->_col = Black;
                }else{
                    RotateLeft(bro);
                    RotateRight(parent);
                    bro->_parent->_col = parent->_col;
                    parent->_col = bro->_col = Black;
                }
            }
        }
        while(_root->_parent) _root = _root->_parent;
        return true;
    }
    // template <class K,class V,class KOfT,class Compare>
    // bool RBTree<K,V,KOfT,Compare>::erase(const K& val) {
    //     Node* cur = _root;
    //     if(_root == nullptr) return false;
    //     while(cur) {
    //         if(_cmp(val,cur->_kv.first)) cur = cur->_left;
    //         else if(_cmp(cur->_kv.first,val))cur = cur->_right;
    //         else break;
    //     }
    //     if(cur == nullptr) return false;//由于cur是因为nullptr而退出的时候就说明我们没有找到
    //     Node* parent = cur->_parent;
    //     //删除分为双孩子和非双孩子
    //     if(cur->_left == nullptr || cur->_right == nullptr) {
    //         Node* child = cur->_left == nullptr ? cur->_right : cur->_left;
    //         if(parent) {
    //             if(cur == parent->_left) parent->_left = child;
    //             else parent->_right = child;
    //         }else _root = child;
    //         if(child) child->_parent = parent;
    //         delete cur;
    //         cur = child;
    //     }else {
    //         Node* rightMin = cur->_right;
    //         while(rightMin->_left) {
    //             rightMin = rightMin->_left;
    //         }//找到后继结点
    //         cur->_kv = rightMin->_kv;//从这里开始实际上就是删除rightMin节点
    //         parent = rightMin->_parent;
    //         Node* child = rightMin->_right;
    //
    //         if(rightMin == parent->_left) parent->_left = child;
    //         else parent->_right = child;
    //         if(child) child->_parent = parent;
    //         delete rightMin;
    //         cur = child;
    //     }
    //
    // }
    template<class K, class V,class KOfT, class Compare>
    void RBTree<K, V,KOfT, Compare>::RotateLeft(Node *parent) {
        //左旋的原理
        //由于某个根节点的平衡因子本身处于1的状态，这个时候如果我们在右子树进行push的时候
        //平衡被打破，进行右旋，将rightChild的left作为parent的rightchild并且将parent作为
        //rightChild的左节点同时将rightchild作为新的根节点
        //
        //右旋我们要考虑几个关键的点
        //1.设置的时候不要忘了我们这里是三叉链的结构
        //2.rCLeftChild可能是nullptr
        //3.旋转之后,parent 和 rightChild 的 balancefactor都应该变成0
        Node* tmp = parent->_parent;//记录根节点
        Node* rightChild = parent->_right;
        Node* rCLeftChild = rightChild->_left;
        parent->_right = rCLeftChild;
        rightChild->_left = parent;
        parent->_parent = rightChild;
        if(rCLeftChild) rCLeftChild->_parent = parent;//由于rCleftChild可能为空
        rightChild->_parent = tmp;//他的parent是parent的parent
        if(tmp) {
            if(parent == tmp->_left) tmp->_left = rightChild;
            else tmp->_right = rightChild;
        }else _root = rightChild;//这个位置也很重要，更新root节点别忘了
    }
    template<class K, class V,class KOfT, class Compare>
    void RBTree<K, V,KOfT, Compare>::RotateRight(Node *parent) {
        Node* tmp = parent->_parent;
        Node* leftChild = parent->_left;
        Node* lCRightChild = leftChild->_right;
        parent->_left = lCRightChild;
        leftChild->_right = parent;
        parent->_parent = leftChild;
        if(lCRightChild) lCRightChild->_parent = parent;
        leftChild->_parent = tmp;
        if(tmp) {
            if(parent == tmp->_left) tmp->_left = leftChild;
            else tmp->_right = leftChild;
        }else _root = leftChild;
    }
}
#endif //RBTREE_H
