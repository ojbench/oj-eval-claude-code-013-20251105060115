/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;

private:
   // Red-Black Tree node structure
   struct Node {
       value_type data;
       Node *left, *right, *parent;
       bool color; // true for red, false for black

       Node(const value_type &val, Node *p = nullptr)
           : data(val), left(nullptr), right(nullptr), parent(p), color(true) {}
   };

   Node *root;
   Node *end_node; // sentinel node for end()
   size_t map_size;
   Compare comp;

   // Helper functions
   void leftRotate(Node *x) {
       Node *y = x->right;
       x->right = y->left;
       if (y->left != nullptr) {
           y->left->parent = x;
       }
       y->parent = x->parent;
       if (x->parent == nullptr) {
           root = y;
       } else if (x == x->parent->left) {
           x->parent->left = y;
       } else {
           x->parent->right = y;
       }
       y->left = x;
       x->parent = y;
   }

   void rightRotate(Node *x) {
       Node *y = x->left;
       x->left = y->right;
       if (y->right != nullptr) {
           y->right->parent = x;
       }
       y->parent = x->parent;
       if (x->parent == nullptr) {
           root = y;
       } else if (x == x->parent->right) {
           x->parent->right = y;
       } else {
           x->parent->left = y;
       }
       y->right = x;
       x->parent = y;
   }

   void fixInsert(Node *z) {
       while (z->parent != nullptr && z->parent->color) {
           if (z->parent == z->parent->parent->left) {
               Node *y = z->parent->parent->right;
               if (y != nullptr && y->color) {
                   z->parent->color = false;
                   y->color = false;
                   z->parent->parent->color = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->right) {
                       z = z->parent;
                       leftRotate(z);
                   }
                   z->parent->color = false;
                   z->parent->parent->color = true;
                   rightRotate(z->parent->parent);
               }
           } else {
               Node *y = z->parent->parent->left;
               if (y != nullptr && y->color) {
                   z->parent->color = false;
                   y->color = false;
                   z->parent->parent->color = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->left) {
                       z = z->parent;
                       rightRotate(z);
                   }
                   z->parent->color = false;
                   z->parent->parent->color = true;
                   leftRotate(z->parent->parent);
               }
           }
       }
       root->color = false;
   }

   void transplant(Node *u, Node *v) {
       if (u->parent == nullptr) {
           root = v;
       } else if (u == u->parent->left) {
           u->parent->left = v;
       } else {
           u->parent->right = v;
       }
       if (v != nullptr) {
           v->parent = u->parent;
       }
   }

   Node *minimum(Node *x) const {
       while (x->left != nullptr) {
           x = x->left;
       }
       return x;
   }

   Node *maximum(Node *x) const {
       if (x == nullptr) return nullptr;
       while (x->right != nullptr) {
           x = x->right;
       }
       return x;
   }

   void fixDelete(Node *x) {
       while (x != root && (x == nullptr || !x->color)) {
           if (x == x->parent->left) {
               Node *w = x->parent->right;
               if (w->color) {
                   w->color = false;
                   x->parent->color = true;
                   leftRotate(x->parent);
                   w = x->parent->right;
               }
               if ((w->left == nullptr || !w->left->color) &&
                   (w->right == nullptr || !w->right->color)) {
                   w->color = true;
                   x = x->parent;
               } else {
                   if (w->right == nullptr || !w->right->color) {
                       if (w->left != nullptr) w->left->color = false;
                       w->color = true;
                       rightRotate(w);
                       w = x->parent->right;
                   }
                   w->color = x->parent->color;
                   x->parent->color = false;
                   if (w->right != nullptr) w->right->color = false;
                   leftRotate(x->parent);
                   x = root;
               }
           } else {
               Node *w = x->parent->left;
               if (w->color) {
                   w->color = false;
                   x->parent->color = true;
                   rightRotate(x->parent);
                   w = x->parent->left;
               }
               if ((w->right == nullptr || !w->right->color) &&
                   (w->left == nullptr || !w->left->color)) {
                   w->color = true;
                   x = x->parent;
               } else {
                   if (w->left == nullptr || !w->left->color) {
                       if (w->right != nullptr) w->right->color = false;
                       w->color = true;
                       leftRotate(w);
                       w = x->parent->left;
                   }
                   w->color = x->parent->color;
                   x->parent->color = false;
                   if (w->left != nullptr) w->left->color = false;
                   rightRotate(x->parent);
                   x = root;
               }
           }
       }
       if (x != nullptr) x->color = false;
   }

   void destroy(Node *node) {
       if (node == nullptr) return;
       destroy(node->left);
       destroy(node->right);
       delete node;
   }

   Node *copy(Node *node, Node *parent) {
       if (node == nullptr) return nullptr;
       Node *new_node = new Node(node->data, parent);
       new_node->color = node->color;
       new_node->left = copy(node->left, new_node);
       new_node->right = copy(node->right, new_node);
       return new_node;
   }

   Node *findNode(const Key &key) const {
       Node *current = root;
       while (current != nullptr) {
           if (comp(key, current->data.first)) {
               current = current->left;
           } else if (comp(current->data.first, key)) {
               current = current->right;
           } else {
               return current;
           }
       }
       return nullptr;
   }

   Node *insertNode(const value_type &value) {
       Node *y = nullptr;
       Node *x = root;

       while (x != nullptr) {
           y = x;
           if (comp(value.first, x->data.first)) {
               x = x->left;
           } else if (comp(x->data.first, value.first)) {
               x = x->right;
           } else {
               return x; // key already exists
           }
       }

       Node *z = new Node(value, y);
       if (y == nullptr) {
           root = z;
       } else if (comp(value.first, y->data.first)) {
           y->left = z;
       } else {
           y->right = z;
       }

       fixInsert(z);
       map_size++;
       return z;
   }

   void deleteNode(Node *z) {
       if (z == nullptr) return;

       Node *y = z;
       Node *x;
       bool y_original_color = y->color;

       if (z->left == nullptr) {
           x = z->right;
           transplant(z, z->right);
       } else if (z->right == nullptr) {
           x = z->left;
           transplant(z, z->left);
       } else {
           y = minimum(z->right);
           y_original_color = y->color;
           x = y->right;
           if (y->parent == z) {
               if (x != nullptr) x->parent = y;
           } else {
               transplant(y, y->right);
               y->right = z->right;
               y->right->parent = y;
           }
           transplant(z, y);
           y->left = z->left;
           y->left->parent = y;
           y->color = z->color;
       }

       if (!y_original_color && x != nullptr) {
           fixDelete(x);
       }

       delete z;
       map_size--;
   }

public:
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   class const_iterator;
   class iterator {
   private:
       Node *current;
       const map *container;

   public:
       iterator() : current(nullptr), container(nullptr) {}

       iterator(Node *node, const map *cont) : current(node), container(cont) {}

       iterator(const iterator &other) : current(other.current), container(other.container) {}

       /**
    * TODO iter++
        */
       iterator operator++(int) {
           iterator temp = *this;
           ++(*this);
           return temp;
       }

       /**
    * TODO ++iter
        */
       iterator &operator++() {
           if (current == nullptr) throw invalid_iterator();

           if (current->right != nullptr) {
               current = container->minimum(current->right);
           } else {
               Node *parent = current->parent;
               while (parent != nullptr && current == parent->right) {
                   current = parent;
                   parent = parent->parent;
               }
               current = parent;
           }

           return *this;
       }

       /**
    * TODO iter--
        */
       iterator operator--(int) {
           iterator temp = *this;
           --(*this);
           return temp;
       }

       /**
    * TODO --iter
        */
       iterator &operator--() {
           if (current == nullptr) {
               // --end() should give the last element
               if (container->root == nullptr) throw invalid_iterator();
               current = container->maximum(container->root);
               return *this;
           }

           if (current->left != nullptr) {
               current = container->maximum(current->left);
           } else {
               Node *parent = current->parent;
               while (parent != nullptr && current == parent->left) {
                   current = parent;
                   parent = parent->parent;
               }
               current = parent;
           }

           // If we reached nullptr after decrementing from the first element, throw
           if (current == nullptr) throw invalid_iterator();
           return *this;
       }

       /**
    * a operator to check whether two iterators are same (pointing to the same memory).
        */
       value_type &operator*() const {
           if (current == nullptr) throw invalid_iterator();
           return current->data;
       }

       bool operator==(const iterator &rhs) const {
           return current == rhs.current && container == rhs.container;
       }

       bool operator==(const const_iterator &rhs) const {
           return current == rhs.current && container == rhs.container;
       }

       /**
    * some other operator for iterator.
        */
       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       /**
    * for the support of it->first.
    * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
        */
       value_type *operator->() const {
           if (current == nullptr) throw invalid_iterator();
           return &(current->data);
       }

       friend class map;
       friend class const_iterator;
   };

   class const_iterator {
   private:
       const Node *current;
       const map *container;

   public:
       const_iterator() : current(nullptr), container(nullptr) {}

       const_iterator(const Node *node, const map *cont) : current(node), container(cont) {}

       const_iterator(const const_iterator &other) : current(other.current), container(other.container) {}

       const_iterator(const iterator &other) : current(other.current), container(other.container) {}

       const_iterator &operator++() {
           if (current == nullptr) throw invalid_iterator();

           if (current->right != nullptr) {
               current = container->minimum(current->right);
           } else {
               const Node *parent = current->parent;
               while (parent != nullptr && current == parent->right) {
                   current = parent;
                   parent = parent->parent;
               }
               current = parent;
           }

           return *this;
       }

       const_iterator operator++(int) {
           const_iterator temp = *this;
           ++(*this);
           return temp;
       }

       const_iterator &operator--() {
           if (current == nullptr) {
               // --end() should give the last element
               if (container->root == nullptr) throw invalid_iterator();
               current = container->maximum(container->root);
               return *this;
           }

           if (current->left != nullptr) {
               current = container->maximum(current->left);
           } else {
               const Node *parent = current->parent;
               while (parent != nullptr && current == parent->left) {
                   current = parent;
                   parent = parent->parent;
               }
               current = parent;
           }

           if (current == nullptr) throw invalid_iterator();
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator temp = *this;
           --(*this);
           return temp;
       }

       const value_type &operator*() const {
           if (current == nullptr) throw invalid_iterator();
           return current->data;
       }

       const value_type *operator->() const {
           if (current == nullptr) throw invalid_iterator();
           return &(current->data);
       }

       bool operator==(const const_iterator &rhs) const {
           return current == rhs.current && container == rhs.container;
       }

       bool operator==(const iterator &rhs) const {
           return current == rhs.current && container == rhs.container;
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       friend class map;
   };


   /**
  * TODO two constructors
    */
   map() : root(nullptr), end_node(nullptr), map_size(0) {}

   map(const map &other) : map_size(other.map_size) {
       root = copy(other.root, nullptr);
       end_node = nullptr;
   }

   /**
  * TODO assignment operator
    */
   map &operator=(const map &other) {
       if (this == &other) return *this;

       destroy(root);

       root = copy(other.root, nullptr);
       map_size = other.map_size;

       return *this;
   }

   /**
  * TODO Destructors
    */
   ~map() {
       destroy(root);
   }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
   T &at(const Key &key) {
       Node *node = findNode(key);
       if (node == nullptr) throw index_out_of_bound();
       return node->data.second;
   }

   const T &at(const Key &key) const {
       const Node *node = findNode(key);
       if (node == nullptr) throw index_out_of_bound();
       return node->data.second;
   }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
   T &operator[](const Key &key) {
       Node *node = findNode(key);
       if (node == nullptr) {
           value_type new_pair(key, T());
           node = insertNode(new_pair);
       }
       return node->data.second;
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const {
       return at(key);
   }

   /**
  * return a iterator to the beginning
    */
   iterator begin() {
       if (root == nullptr) return iterator(nullptr, this);
       return iterator(minimum(root), this);
   }

   const_iterator cbegin() const {
       if (root == nullptr) return const_iterator(nullptr, this);
       return const_iterator(minimum(root), this);
   }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
   iterator end() {
       return iterator(nullptr, this);
   }

   const_iterator cend() const {
       return const_iterator(nullptr, this);
   }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
   bool empty() const {
       return map_size == 0;
   }

   /**
  * returns the number of elements.
    */
   size_t size() const {
       return map_size;
   }

   /**
  * clears the contents
    */
   void clear() {
       destroy(root);
       root = nullptr;
       map_size = 0;
   }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
   pair<iterator, bool> insert(const value_type &value) {
       Node *node = findNode(value.first);
       if (node != nullptr) {
           return pair<iterator, bool>(iterator(node, this), false);
       }

       node = insertNode(value);
       return pair<iterator, bool>(iterator(node, this), true);
   }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
   void erase(iterator pos) {
       if (pos.container != this || pos.current == nullptr) {
           throw invalid_iterator();
       }

       deleteNode(pos.current);
   }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
   size_t count(const Key &key) const {
       return findNode(key) != nullptr ? 1 : 0;
   }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
   iterator find(const Key &key) {
       Node *node = findNode(key);
       if (node == nullptr) return end();
       return iterator(node, this);
   }

   const_iterator find(const Key &key) const {
       const Node *node = findNode(key);
       if (node == nullptr) return cend();
       return const_iterator(node, this);
   }
};

}

#endif