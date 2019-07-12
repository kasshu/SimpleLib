#ifndef SIMPLELIB_RED_BLACK_TREE_HPP_
#define SIMPLELIB_RED_BLACK_TREE_HPP_

#include <functional>
#include "common.h"

BEGIN_NAMESPACE_SIMPLELIB

//The red-black tree
template <typename Key, typename Value, typename Comp = std::less<Key>>
class RedBlackTree {
private:
//************************* Internal structures ******************************

    //Internal enum class Color, user won't care about this
    enum class Color { RED, BLACK };

    //Internal struct Node, user won't care about this
    struct Node {
        //Default constructor used by sentinel
        Node(): _c(Color::BLACK) {}

        //Constructor used by insert node
        Node(const Key &key, const Value &value): _k(key), _v(value) {}

        //Data area
        Key _k = Key();
        Value _v = Value();
        Color _c = Color::RED;
        Node *_left = nullptr;
        Node *_right = nullptr;
        Node *_parent = nullptr;
    };

//******************************* Rotations **********************************

    //Left rotate:
    //          node                           temp
    //        /     \                         /     \
    //       x       temp        -->      node       z
    //              /    \               /    \
    //             y      z             x      y
    //
    void left_rotate(Node *node) {
        Node *temp = node->_right;
        node->_right = temp->_left;
        if (temp->_left != _sentinel) {
            temp->_left->_parent = node;
        }
        temp->_parent = node->_parent;
        if (node->_parent == _sentinel) {
            _root = temp;
        } else if (node == node->_parent->_left) {
            node->_parent->_left = temp;
        } else {
            node->_parent->_right = temp;
        }
        temp->_left = node;
        node->_parent = temp;
    }

    //Right rotate:
    //          node                           temp
    //        /     \                         /     \
    //    temp       z           -->         x      node
    //   /    \                                    /    \
    //  x      y                                  y      z
    //
    void right_rotate(Node *node) {
        Node *temp = node->_left;
        node->_left = temp->_right;
        if (temp->_right != _sentinel) {
            temp->_right->_parent = node;
        }
        temp->_parent = node->_parent;
        if (node->_parent == _sentinel) {
            _root = temp;
        } else if (node == node->_parent->_left) {
            node->_parent->_left = temp;
        } else {
            node->_parent->_right = temp;
        }
        temp->_right = node;
        node->_parent = temp;
    }

//***************************** Insertion ************************************
    //Insertion fixup
    //case 1~3 are symmetric with case 4~6
    //
    //case 1:
    //
    //         x(B)                       x(R)   <--continue with this
    //      /       \                   /      \
    //  y(R)        temp(R)   ->    y(B)        temp(B)
    //      \                           \
    //       node(R)                     node(R)
    //
    //                x(B)                       x(R)   <--continue with this
    //             /       \                   /      \
    //         y(R)        temp(R)   ->    y(B)        temp(B)
    //        /                          /
    // node(R)                     node(R)
    //
    //case 2:
    //
    //         x(B)                           x(B)
    //      /       \                       /      \
    //  y(R)        temp(B)   ->     node(R)        temp(B)
    //      \                          /
    //       node(R)               y(R) <--go to case 3 with this
    //
    //case 3:
    //
    //            x(B)                        node(B)
    //          /      \                    /        \
    //   node(R)        temp(B)   ->    y(R)          x(R)
    //     /                                              \
    // y(R)                                                temp(B)
    //
    //
    void insert_fixup(Node *node) {
        while (node->_parent->_c == Color::RED) {
            if (node->_parent == node->_parent->_parent->_left) {
                Node *temp = node->_parent->_parent->_right;
                if (temp->_c == Color::RED) {                 //case 1
                    node->_parent->_c = Color::BLACK;
                    temp->_c = Color::BLACK;
                    node->_parent->_parent->_c = Color::RED;
                    node = node->_parent->_parent;
                } else {
                    if (node == node->_parent->_right) {      //case 2
                        node = node->_parent;
                        left_rotate(node);
                    }
                    node->_parent->_c = Color::BLACK;         //case 3
                    node->_parent->_parent->_c = Color::RED;
                    right_rotate(node->_parent->_parent);
                }
            } else {
                Node *temp = node->_parent->_parent->_left;
                if (temp->_c == Color::RED) {                 //case 4
                    node->_parent->_c = Color::BLACK;
                    temp->_c = Color::BLACK;
                    node->_parent->_parent->_c = Color::RED;
                    node = node->_parent->_parent;
                } else {
                    if (node == node->_parent->_left) {       //case 5
                        node = node->_parent;
                        right_rotate(node);
                    }
                    node->_parent->_c = Color::BLACK;         //case 6
                    node->_parent->_parent->_c = Color::RED;
                    left_rotate(node->_parent->_parent);
                }
            }
        }
        _root->_c = Color::BLACK;
    }

    void inner_insert(const Key& k, const Value& v, Node *parent) {
        Node *node = new Node(k, v);

        if (parent == _sentinel) {
            _root = node;
        } else if (_comp(node->_k, parent->_k)) {
            parent->_left = node;
        } else {
            parent->_right = node;
        }

        //Assign sentinels
        node->_left = _sentinel;
        node->_right = _sentinel;
        node->_parent = parent;

        insert_fixup(node);
    }

//******************************* Successor **********************************

    Node * successor(Node *node) {
        Node *ret = _sentinel;
        if (node->_right != _sentinel) {
            ret = node->_right;
            while (ret->_left != _sentinel) {
                ret = ret->_left;
            }
        } else {
            ret = node->_parent;
            while (ret != _sentinel && node == ret->_right) {
                node = ret;
                ret = ret->_parent;
            }
        }
        return ret;
    }

//******************************* Finder *************************************

    //Finder: return the position for insertion if the key does not exist
    Node * inner_find(const Key &k) {
        Node *temp = _root;
        Node *parent = _sentinel;
        while (temp != _sentinel) {
            parent = temp;
            if (_comp(k, temp->_k)) {
                temp = temp->_left;
            } else if (_comp(temp->_k, k)) {
                temp = temp->_right;
            } else {
                break;
            }
        }
        return parent;
    }

//******************************* Deletion ***********************************

    //Deletion fixup
    //case 1~4 are symmetric with case 5~8
    //the node which node points has an extra black
    //
    //case 1:
    //
    //          x(B)                             temp(B)
    //        /       \                         /      \
    // node(B)         temp(R)      ->      x(R)        z(B)
    //                /      \             /   \
    //            y(B)       z(B)     node(B)   y(B) <--new temp
    //
    //case 2:
    //
    //          x(c)                              x(c) <--new node
    //        /       \                         /      \
    // node(B)         temp(B)      ->     node(B)      temp(R)
    //                /      \                         /       \
    //            y(B)       z(B)                   y(B)       z(B)
    //
    //case 3:
    //
    //          x(c)                              x(c)
    //        /       \                         /      \
    // node(B)         temp(B)      ->     node(B)      y(B) <--new temp
    //                /      \                              \
    //            y(R)       z(B)                            temp(R)
    //                                                             \
    //                                                              z(B)
    //
    //case 4:
    //
    //          x(c)                                temp(c)
    //        /      \                             /       \
    // node(B)       temp(B)        ->          x(B)        z(B)
    //              /       \                  /   \
    //          y(c')        z(R)       node(B)     y(c')
    //

    void delete_fixup(Node *node) {
        Node *temp = nullptr;
        while (node != _root && node->_c == Color::BLACK) {
            if (node == node->_parent->_left) {
                temp = node->_parent->_right;
                if (temp->_c == Color::RED) {               //case 1
                    temp->_c = Color::BLACK;
                    node->_parent->_c = Color::RED;
                    left_rotate(node->_parent);
                    temp = node->_parent->_right;
                }
                if (temp->_left->_c == Color::BLACK &&
                    temp->_right->_c == Color::BLACK) {     //case 2
                    temp->_c = Color::RED;
                    node = node->_parent;
                } else {
                    if (temp->_right->_c == Color::BLACK) { //case 3
                        temp->_left->_c = Color::BLACK;
                        temp->_c = Color::RED;
                        right_rotate(temp);
                        temp = node->_parent->_right;
                    }
                    if (temp == _sentinel) {
                        printf("here");
                    }
                    temp->_c = node->_parent->_c;           //case 4
                    node->_parent->_c = Color::BLACK;
                    temp->_right->_c = Color::BLACK;
                    left_rotate(node->_parent);
                    node = _root;
                }
            } else {
                temp = node->_parent->_left;
                if (temp->_c == Color::RED) {               //case 5
                    temp->_c = Color::BLACK;
                    node->_parent->_c = Color::RED;
                    right_rotate(node->_parent);
                    temp = node->_parent->_left;
                }
                if (temp->_left->_c == Color::BLACK &&
                    temp->_right->_c == Color::BLACK) {     //case 6
                    temp->_c = Color::RED;
                    node = node->_parent;
                } else {
                    if (temp->_left->_c == Color::BLACK) {  //case 7
                        temp->_right->_c = Color::BLACK;
                        temp->_c = Color::RED;
                        left_rotate(temp);
                        temp = node->_parent->_left;
                    }
                    temp->_c = node->_parent->_c;           //case 8
                    node->_parent->_c = Color::BLACK;
                    temp->_left->_c = Color::BLACK;
                    right_rotate(node->_parent);
                    node = _root;
                }
            }
        }
        node->_c = Color::BLACK;
    }

    void inner_delete(Node *node) {
        //Find the node p to be deleted
        Node *p = _sentinel;
        if (node->_left == _sentinel || node->_right == _sentinel) {
            p = node;
        } else {
            p = successor(node);
        }

        //Modify nodes around p
        Node *q = _sentinel;
        if (p->_left != _sentinel) {
            q = p->_left;
        } else {
            q = p->_right;
        }

        q->_parent = p->_parent;

        if (p->_parent == _sentinel) {
            _root = q;
        } else if (p == p->_parent->_left) {
            p->_parent->_left = q;
        } else {
            p->_parent->_right = q;
        }

        //Copy k/v to node
        if (p != node) {
            node->_k = p->_k;
            node->_v = p->_v;
        }

        if (p->_c == Color::BLACK) {
            delete_fixup(q);
        }

        delete p;
        p = nullptr;
    }

//****************************** Cleanup *************************************

    //Destroy the tree using pre-order traverse
    void inner_destroy(Node *node) {
        if (node->_left != _sentinel) {
            inner_destroy(node->_left);
        }
        if (node->_right != _sentinel) {
            inner_destroy(node->_right);
        }
        delete node;
    }

#ifdef UNIT_TEST
//Check the balance of black height by post-order traverse
int inner_check_balance(Node *node) {
    if (node == _sentinel) { //When tree is empty
        return 0;
    }

    int black_height_left = 0;
    int black_height_right = 0;

    if (node->_left != _sentinel) {
        black_height_left = inner_check_balance(node->_left);
        //Imbalance found in left subtree
        if (black_height_left < 0) {
            return -1;
        }
    }

    if (node->_right != _sentinel) {
        black_height_right = inner_check_balance(node->_right);
        //Imbalance found in left subtree
        if (black_height_right < 0) {
            return -1;
        }
    }

    int black_height_delta = node->_c == Color::BLACK ? 1 : 0;
    if (node->_left != _sentinel && node->_right != _sentinel) {
        //Imbalance found
        if (black_height_left != black_height_right) {
            return -1;
        } else {
            return black_height_left + black_height_delta;
        }
    } else if (node->_left != _sentinel) {
        return black_height_left + black_height_delta;
    } else if (node->_right != _sentinel) {
        return black_height_right + black_height_delta;
    } else {
        return black_height_delta;
    }
}
#endif

//****************************** Data area ***********************************
    Comp _comp = Comp();
    Node *_sentinel = nullptr;
    Node *_root = nullptr;
    uint32_t _size = 0;

public:
//****************** Constructors and Deconstructor **************************

    RedBlackTree() {
        _sentinel = new Node();
        _root = _sentinel;
    }

    virtual ~RedBlackTree() {
        tree_clear();
        if (_sentinel != nullptr) {
            delete _sentinel;
        }
    }

//***************************** Interfaces ***********************************

    bool tree_insert(const Key& k, const Value& v) {
        Node *target = inner_find(k);
        if (target != _sentinel && target->_k == k) {
            return false;
        }

        inner_insert(k, v, target);
        _size++;

        return true;
    }

    bool tree_delete(const Key& k) {
        Node *target = inner_find(k);
        if (target == _sentinel || target->_k != k) {
            return false;
        }

        inner_delete(target);
        _size--;

        return true;
    }

    bool tree_find(const Key& k, Value *v) {
        Node *target = inner_find(k);
        if (target == _sentinel || target->_k != k) {
            return false;
        }

        *v = target->_v;
        return true;
    }

    void tree_clear() {
        if (_root != _sentinel) {
            inner_destroy(_root);
        }
        _root = _sentinel;
        _size = 0;
    }

    uint32_t tree_size() {
        return _size;
    }

#ifdef UNIT_TEST
    bool check_balanced() {
        return inner_check_balance(_root) < 0 ? false : true;
    }
#endif
};

END_NAMESPACE_SIMPLELIB

#endif  //SIMPLELIB_RED_BLACK_TREE_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
