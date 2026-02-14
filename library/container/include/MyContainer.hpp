#pragma once

#include <memory>
#include <iterator>
#include <cstddef>
#include <utility>
#include <cassert>

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

    struct Node;

    using allocator_traits_t = std::allocator_traits<Allocator>;

    template<typename U>
    using rebind_alloc_t =
            typename allocator_traits_t::template rebind_alloc<U>;

    using node_allocator_t = rebind_alloc_t<Node>;
    using node_traits_t    = std::allocator_traits<node_allocator_t>;
    using node_pointer     = typename node_traits_t::pointer;
    using node_ptr_traits  = std::pointer_traits<node_pointer>;

    struct Node {
        T value;
        node_pointer next;

        Node() = default;

        template<typename... Args>
        explicit Node(node_pointer n, Args&&... args)
                : value(std::forward<Args>(args)...), next(n) {}
    };


public:
    /**
     * @brief Forward iterator for MyContainer
     *
     * Provides read/write access to elements.
     * Models a standard forward iterator.
     */
    class iterator {
        node_pointer cur_ = nullptr;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        iterator() = default;
        explicit iterator(node_pointer p) : cur_(p) {}

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
        sentinel_ptr_ = node_ptr_traits::pointer_to(sentinel_);
        std::to_address(sentinel_ptr_)->next = sentinel_ptr_;

        head_ = tail_ = sentinel_ptr_;
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
    iterator end() noexcept { return iterator(sentinel_ptr_); }

    /// Checks whether the container is empty
    [[nodiscard]] bool empty() const noexcept {
        return head_ == sentinel_ptr_;
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
        node_pointer n = create_node(value);

        if (empty()) {
            std::to_address(n)->next = sentinel_ptr_;
            head_ = tail_ = n;
        } else {
            std::to_address(n)->next = head_;
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
        node_pointer n = create_node(value);
        std::to_address(n)->next = sentinel_ptr_;

        if (empty()) {
            head_ = tail_ = n;
        } else {
            std::to_address(tail_)->next = n;
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

        node_pointer old = head_;
        head_ = std::to_address(head_)->next;

        if (head_ == sentinel_ptr_)
            tail_ = sentinel_ptr_;

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

    MyContainer(const MyContainer& other)
            : node_alloc_(node_traits_t::select_on_container_copy_construction(other.node_alloc_))
    {
        sentinel_ptr_ = node_ptr_traits::pointer_to(sentinel_);
        std::to_address(sentinel_ptr_)->next = sentinel_ptr_;

        head_ = tail_ = sentinel_ptr_;
        size_ = 0;

        for (auto it = other.begin(); it != other.end(); ++it) {
            push_back(*it);
        }
    }

    MyContainer(MyContainer&& other) noexcept
            : node_alloc_(std::move(other.node_alloc_))
    {
        sentinel_ptr_ = node_ptr_traits::pointer_to(sentinel_);
        std::to_address(sentinel_ptr_)->next = sentinel_ptr_;

        if (node_alloc_ == other.node_alloc_) {
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;

            other.head_ = other.tail_ = other.sentinel_ptr_;
            other.size_ = 0;
        } else {
            head_ = tail_ = sentinel_ptr_;
            size_ = 0;

            for (auto& v : other)
                push_back(std::move(v));

            other.clear();
        }
    }

    MyContainer& operator=(MyContainer&& other) noexcept(
    node_traits_t::propagate_on_container_move_assignment::value ||
            node_traits_t::is_always_equal::value)
    {
        if (this == &other)
            return *this;

        clear();

        if constexpr (node_traits_t::propagate_on_container_move_assignment::value) {
            node_alloc_ = std::move(other.node_alloc_);

            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;

            other.head_ = other.tail_ = other.sentinel_ptr_;
            other.size_ = 0;
        }
        else {
            if (node_alloc_ == other.node_alloc_) {

                head_ = other.head_;
                tail_ = other.tail_;
                size_ = other.size_;

                other.head_ = other.tail_ = other.sentinel_ptr_;
                other.size_ = 0;
            }
            else {
                for (auto& v : other)
                    push_back(std::move(v));

                other.clear();
            }
        }

        return *this;
    }

    MyContainer& operator=(const MyContainer& other)
    {
        if (this == &other)
            return *this;

        if constexpr (node_traits_t::propagate_on_container_copy_assignment::value) {

            clear();
            node_alloc_ = other.node_alloc_;

            for (const auto& v : other)
                push_back(v);
        }
        else {

            if (node_alloc_ == other.node_alloc_) {

                clear();
                for (const auto& v : other)
                    push_back(v);
            }
            else {

                MyContainer tmp(other);
                swap(tmp);
            }
        }

        return *this;
    }


    void swap(MyContainer& other) noexcept(
    node_traits_t::propagate_on_container_swap::value ||
    node_traits_t::is_always_equal::value)
    {
        using traits = node_traits_t;

        if (this == &other)
            return;

        if constexpr (traits::propagate_on_container_swap::value) {

            std::swap(node_alloc_, other.node_alloc_);
            std::swap(head_, other.head_);
            std::swap(tail_, other.tail_);
            std::swap(size_, other.size_);
        }
        else {

            if (node_alloc_ == other.node_alloc_) {

                std::swap(head_, other.head_);
                std::swap(tail_, other.tail_);
                std::swap(size_, other.size_);
            }
            else {
                // по стандарту это UB
                // можно добавить assert
                assert(false && "Swapping containers with unequal allocators is undefined");
            }
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
    node_pointer create_node(Args&&... args) {
        node_pointer p = node_traits_t::allocate(node_alloc_, 1);
        Node* raw = std::to_address(p);
        try {
            node_traits_t::construct(node_alloc_, raw, node_pointer{}, args...);
            p->next = nullptr;
        } catch (...) {
            node_traits_t::deallocate(node_alloc_, raw, 1);
            throw;
        }
        return p;
    }

    /**
     * @brief Destroys and deallocates a node
     *
     * @param p Node to destroy
     */
    void destroy_node(node_pointer p) noexcept {
        Node* raw = std::to_address(p);
        node_traits_t::destroy(node_alloc_, raw);
        node_traits_t::deallocate(node_alloc_, p, 1);
    }

private:

    node_allocator_t node_alloc_;
    Node sentinel_{};
    node_pointer sentinel_ptr_{};

    node_pointer head_{};
    node_pointer tail_{};

    std::size_t size_ = 0;
};
