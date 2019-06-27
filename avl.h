#ifndef AVL_H_
#define AVL_H_

#include <cstddef>
#include <utility>
#include <algorithm>
#include <initializer_list>

namespace avl {

template<class ValueType>
class Set {
 private:
    struct Node {
        ValueType key;
        Node *L, *R, *P;
        size_t height;
        size_t size;

        explicit Node(ValueType key):
        key(std::move(key)),
        L(nullptr),
        R(nullptr),
        P(nullptr),
        height(1),
        size(1) {}
    } *root;

    static size_t get_height(const Node *N) {
        return (N ? N->height : 0);
    }

    static size_t get_size(const Node *N) {
        return (N ? N->size : 0);
    }

    static void recalc_node(Node *N) {
        if (!N)
            return;
        N->height = std::max(get_height(N->L), get_height(N->R)) + 1;
        N->size = get_size(N->L) + get_size(N->R) + 1;
        recalc_par(N);
    }

    static void recalc_par(Node *N) {
        if (!N)
            return;
        if (N->L) {
            N->L->P = N;
        }
        if (N->R) {
            N->R->P = N;
        }
    }

    static Node *left_rotate(Node *N) {
        Node *C = N->R;
        Node *T = C->L;

        C->L = N;
        N->R = T;

        C->P = N->P;

        recalc_node(N);
        recalc_node(C);

        return C;
    }

    static Node *right_rotate(Node *N) {
        Node *C = N->L;
        Node *T = C->R;

        C->R = N;
        N->L = T;

        C->P = N->P;

        recalc_node(N);
        recalc_node(C);

        return C;
    }

    static int get_balance(const Node *N) {
        int left_height = static_cast<int>(get_height(N->L));
        int right_height = static_cast<int>(get_height(N->R));
        return left_height - right_height;
    }

    static Node *_insert(Node *N, const ValueType& key) {
        if (!N) {
            return new Node(key);
        }

        if (key < N->key) {
            N->L = _insert(N->L, key);
        } else if (N->key < key) {
            N->R = _insert(N->R, key);
        } else {
            return N;
        }

        recalc_node(N);

        int bal = get_balance(N);

        if (bal > 1) {
            if (key < N->L->key) {  // LL
                return right_rotate(N);
            } else {  // LR
                N->L = left_rotate(N->L);
                return right_rotate(N);
            }
        } else if (bal < -1) {
            if (key < N->R->key) {  // RL
                N->R = right_rotate(N->R);
                return left_rotate(N);
            } else {  // RR
                return left_rotate(N);
            }
        }

        return N;
    }

    static Node *get_left(Node *N) {
        if (!N) {
            return nullptr;
        }
        if (!(N->L)) {
            return N;
        }
        return get_left(N->L);
    }

    static Node *get_right(Node *N) {
        if (!N) {
            return nullptr;
        }
        if (!(N->R)) {
            return N;
        }
        return get_right(N->R);
    }

    static Node *get_next(Node *N) {
        if (!N)
            return nullptr;
        if (N->R)
            return get_left(N->R);
        while (N->P) {
            if (N->P->L == N) {
                return N->P;
            }
            N = N->P;
        }
        return nullptr;
    }

    static Node *get_prev(Node *N) {
        if (!N)
            return nullptr;
        if (N->L)
            return get_right(N->L);
        while (N->P) {
            if (N->P->R == N) {
                return N->P;
            }
            N = N->P;
        }
        return nullptr;
    }

    static Node *_erase(Node *N, const ValueType& key) {
        if (!N)
            return nullptr;
        if (key < N->key) {
            N->L = _erase(N->L, key);
        } else if (N->key < key) {
            N->R = _erase(N->R, key);
        } else {
            if (!(N->L) || !(N->R)) {
                Node *T = (N->L ? N->L : N->R);
                if (!T) {
                    T = N;
                    N = nullptr;
                } else {
                    *N = *T;
                }
                delete T;
                T = nullptr;
            } else {
                Node *T = get_left(N->R);
                N->key = T->key;
                N->R = _erase(N->R, T->key);
            }
        }

        if (!N)
            return nullptr;

        recalc_node(N);

        int bal = get_balance(N);

        if (bal > 1) {
            if (get_balance(N->L) >= 0) {
                return right_rotate(N);
            } else {
                N->L = left_rotate(N->L);
                return right_rotate(N);
            }
        } else if (bal < -1) {
            if (get_balance(N->R) <= 0) {
                return left_rotate(N);
            } else {
                N->R = right_rotate(N->R);
                return left_rotate(N);
            }
        }

        return N;
    }

    static Node *_find(Node *N, const ValueType& key) {
        if (!N) {
            return nullptr;
        }
        if (key < N->key) {
            return _find(N->L, key);
        } else if (N->key < key) {
            return _find(N->R, key);
        } else {
            return N;
        }
    }

    static Node *_lower_bound(Node *N, const ValueType& key) {
        if (!N)
            return nullptr;
        if (!(key < N->key) && !(N->key < key))
            return N;
        if (key < N->key) {
            Node *res = _lower_bound(N->L, key);
            if (!res)
                res = N;
            return res;
        }
        return _lower_bound(N->R, key);
    }

    static void destroy(Node *N) {
        if (!N)
            return;
        destroy(N->L);
        destroy(N->R);
        delete N;
    }

 public:
    class iterator {
     private:
        Node *cur;
        Node *root;

     public:
        iterator(): cur(nullptr), root(nullptr) {}
        iterator(Node *N, Node *root): cur(N), root(root) {}

        iterator(const iterator& other) {
            cur = other.cur;
            root = other.root;
        }

        iterator& operator++() {
            cur = get_next(cur);
            return *this;
        }

        iterator& operator--() {
            if (cur == nullptr) {
                cur = get_right(root);
            } else {
                cur = get_prev(cur);
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return cur == other.cur;
        }

        bool operator!=(const iterator& other) const {
            return cur != other.cur;
        }

        ValueType& operator*() const {
            return cur->key;
        }

        ValueType* operator->() const {
            return &(cur->key);
        }
    };

    Set(): root(nullptr) {}

    template<typename Iter>
    Set(Iter start, Iter end): root(nullptr) {
        try {
            while (start != end) {
                root = _insert(root, *start);
                ++start;
            }
        } catch (...) {
            destroy(root);
        }
    }

    Set(std::initializer_list<ValueType> elems): root(nullptr) {
        *this = Set(elems.begin(), elems.end());
    }

    Set(const Set &other): root(nullptr) {
        *this = Set(other.begin(), other.end());
    }

    Set& operator=(const Set &other) {
        if (root == other.root)
            return *this;

        destroy(root);
        root = nullptr;

        auto start = other.begin();
        while (start != other.end()) {
            root = _insert(root, *start);
            ++start;
        }

        return *this;
    }

    iterator begin() const {
        return iterator(get_left(root), root);
    }

    iterator end() const {
        return iterator(nullptr, root);
    }

    void insert(const ValueType &val) {
        root = _insert(root, val);
    }

    void erase(const ValueType &val) {
        root = _erase(root, val);
    }

    iterator find(const ValueType& val) const {
        return iterator(_find(root, val), root);
    }

    iterator lower_bound(const ValueType& val) const {
        return iterator(_lower_bound(root, val), root);
    }

    size_t size() const {
        return get_size(root);
    }

    bool empty() const {
        return (root == nullptr);
    }

    ~Set() {
        destroy(root);
        root = nullptr;
    }
};

}  // namespace avl

#endif  // AVL_H_
