//
//  lockfree_container.h
//  lockfree_container
//
//  Created by Dmitrii Torkhov <dmitriitorkhov@gmail.com> on 07.09.2021.
//  Copyright © 2021 Dmitrii Torkhov. All rights reserved.
//

#pragma once

#include <atomic>
#include <functional>
#include <iterator>

namespace oo {

    template<class T, size_t c_capacity = 16>
    class lockfree_container {
    public:

        class iterator : public std::iterator<std::forward_iterator_tag, T, size_t, T *, const T &> {
        public:

            iterator(lockfree_container *container, size_t index) : m_container(container), m_index(index) {
                find_valid_index_and_acquire();
            }

            ~iterator() {
                release_current_index();
            }

        public:

            T &operator*() const {
                return m_container->m_data[m_index].t;
            }

            iterator &operator++() {
                release_current_index();
                ++m_index;
                find_valid_index_and_acquire();

                return *this;
            }

            bool operator==(const iterator &other) const {
                return m_index == other.m_index;
            }

            bool operator!=(const iterator &other) const {
                return !(operator==(other));
            }

        private:

            lockfree_container *m_container;
            size_t m_index;

        private:

            void find_valid_index_and_acquire() {
                if (m_index == c_capacity) {
                    return;
                }

                for (size_t i = m_index; i < c_capacity; ++i) {
                    if (m_container->m_data[i].empty.acquire()) {
                        m_index = i;
                        return;
                    }
                }

                m_index = c_capacity;
            }

            void release_current_index() {
                if (m_index != c_capacity) {
                    m_container->m_data[m_index].empty.release();
                }
            }

        };

    public:

        iterator begin() {
            return iterator{this, 0};
        }

        iterator end() {
            return iterator{this, c_capacity};
        }

        bool push(const T &t) {
            for (auto &node: m_data) {
                if (node.full.acquire()) {
                    node.t = t;
                    node.empty.release();

                    m_empty.release();
                    return true;
                }
            }

            return false;
        }

        bool pop(T &t) {
            do {
                for (auto &node: m_data) {
                    if (node.empty.acquire()) {
                        t = node.t;
                        node.t = {};
                        node.full.release();

                        return true;
                    }
                }
            } while (m_empty.acquire());

            return false;
        }

        bool insert(const T &t) {
            const auto hash = std::hash<T>{}(t);
            for (size_t i = 0; i < c_capacity; ++i) {
                const auto index = (hash + i) % c_capacity;
                auto &node = m_data[index];

                if (node.full.acquire()) {
                    node.t = t;
                    node.empty.release();

                    m_empty.release();
                    return true;
                } else {
                    if (hash == std::hash<T>{}(node.t)) {
                        return false;
                    }
                }
            }

            return false;
        }

        bool erase(const T &t) {
            const auto hash = std::hash<T>{}(t);
            for (size_t i = 0; i < c_capacity; ++i) {
                const auto index = (hash + i) % c_capacity;
                auto &node = m_data[index];

                if (node.empty.acquire()) {
                    if (hash == std::hash<T>{}(node.t)) {
                        node.t = {};
                        node.full.release();

                        return true;
                    } else {
                        node.empty.release();
                    }
                }
            }

            return false;
        }

    private:

        class lock {
        public:

            lock() {
                m_lock.clear();
            }

        public:

            bool acquire() {
                return !m_lock.test_and_set(std::memory_order_acquire);
            }

            void release() {
                m_lock.clear(std::memory_order_release);
            }

        private:

            std::atomic_flag m_lock{};

        };

    private:

        struct node {

            node() {
                empty.acquire();
            }

            T t;
            lock full;
            lock empty;

        };

    private:

        node m_data[c_capacity]{};
        lock m_empty;

    };

}