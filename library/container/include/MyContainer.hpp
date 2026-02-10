#pragma once

#include <memory>
#include <iterator>
#include <cstddef>
#include <utility>

/**
 * @brief Singly-linked container with allocator support
 *
 * MyContainer is a simple singly-linked container that stores elements
 * in dynamically allocated nodes. Memory management is fully delegated
 * to the provided allocator and performed via std::allocator_traits.
 *
 * Characteristics:
 * - forward iteration only
 * - constant-time insertion at front and back
 * - linear-time destruction
 * - allocator-aware (supports custom allocators)
 *
 * This container:
 * - owns all its elements
 * - does not support random access
 * - does not provide erase-by-iterator
 *
 * @tparam T Value type stored in the container
 * @tparam Allocator Allocator type used for node allocation
 *
 * @note This container is not thread-safe
 * @note Iterators are invalidated on element removal
 */

template<typename T, typename Allocator = std::allocator<T>>
class MyContainer {
private:
    /**
     * @brief Internal node of the singly-linked list
     *
     * Stores the value and a pointer to the next node.
     */
    struct Node {
        T value;
        Node* next = nullptr;
    };

    using allocator_traits_t = std::allocator_traits<Allocator>;

    /**
     * @brief Allocator rebound to Node type
     *
     * Required to allocate memory for internal nodes.
     */
    template<typename U>
    using rebind_alloc_t =
            typename allocator_traits_t::template rebind_alloc<U>;

    using node_allocator_t = rebind_alloc_t<Node>;
    using node_traits_t    = std::allocator_traits<node_allocator_t>;

public:
    /**
     * @brief Forward iterator for MyContainer
     *
     * Provides read/write access to elements.
     * Models a standard forward iterator.
     */
    class iterator {
        Node* cur_ = nullptr;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        iterator() = default;
        explicit iterator(Node* p) : cur_(p) {}

        reference operator*() const noexcept {
            return cur_->value;
        }

        pointer operator->() const noexcept {
            return std::addressof(cur_->value);
        }

        iterator& operator++() noexcept {
            cur_ = cur_->next;
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        friend bool operator==(const iterator& a, const iterator& b) noexcept {
            return a.cur_ == b.cur_;
        }

        friend bool operator!=(const iterator& a, const iterator& b) noexcept {
            return !(a == b);
        }
    };

    /**
     * @brief Constructs an empty container
     *
     * @param alloc Allocator instance used for node allocation
     */
    explicit MyContainer(const Allocator& alloc = Allocator{})
            : node_alloc_(alloc)
    {
        sentinel_.next = &sentinel_;
    }

    /**
     * @brief Destroys the container and all stored elements
     */
    ~MyContainer() {
        clear();
    }

    /// Returns iterator to the first element
    iterator begin() noexcept { return iterator(head_); }

    /// Returns iterator past the last element
    iterator end() noexcept { return iterator(&sentinel_); }

    /// Checks whether the container is empty
    [[nodiscard]] bool empty() const noexcept {
        return head_ == &sentinel_;
    }


    /// Returns number of elements in the container
    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    /**
     * @brief Inserts an element at the front
     *
     * @param value Value to insert
     */
    void push_front(const T& value) {
        Node* n = create_node(value);

        if (empty()) {
            n->next =&sentinel_;
            head_ = tail_ = n;
        } else {
            n->next = head_;
            head_ = n;
        }
        ++size_;
    }

    /**
     * @brief Inserts an element at the back
     *
     * @param value Value to insert
     */
    void push_back(const T& value) {
        Node* n = create_node(value);
        n->next = &sentinel_;
        if (empty()) {
            head_ = tail_ = n;
        } else {
            tail_->next = n;
            tail_ = n;
        }
        ++size_;
    }

    /**
     * @brief Removes the first element
     *
     * Does nothing if the container is empty.
     */
    void pop_front() {
        if (empty())
            return;

        Node* old = head_;
        head_ = head_->next;

        if (head_ == &sentinel_)
            tail_ = &sentinel_;

        destroy_node(old);
        --size_;
    }

    /**
     * @brief Removes all elements from the container
     */
    void clear() noexcept {
        while (!empty()) {
            pop_front();
        }
    }


private:
    /**
     * @brief Allocates and constructs a node
     *
     * @tparam Args Constructor argument types
     * @param args Arguments forwarded to T constructor
     *
     * @return Pointer to newly created node
     *
     * @throws Propagates exceptions from allocation or construction
     */
    template<typename... Args>
    Node* create_node(Args&&... args) {
        Node* p = node_traits_t::allocate(node_alloc_, 1);
        try {
            node_traits_t::construct(
                    node_alloc_,
                    std::addressof(p->value),
                    std::forward<Args>(args)...);
            p->next = nullptr;
        } catch (...) {
            node_traits_t::deallocate(node_alloc_, p, 1);
            throw;
        }
        return p;
    }

    /**
     * @brief Destroys and deallocates a node
     *
     * @param p Node to destroy
     */
    void destroy_node(Node* p) noexcept {
        node_traits_t::destroy(node_alloc_, std::addressof(p->value));
        node_traits_t::deallocate(node_alloc_, p, 1);
    }

private:

    node_allocator_t node_alloc_;
    Node sentinel_{};

    Node* head_ = &sentinel_;
    Node* tail_ = &sentinel_;

    std::size_t size_ = 0;
};
