//
//  lockfree_container.h
//  lockfree_container
//
//  Created by Dmitrii Torkhov <dmitriitorkhov@gmail.com> on 07.09.2021.
//  Copyright © 2021 Dmitrii Torkhov. All rights reserved.
//

#pragma once

#include <atomic>

namespace oo {

    template<class T, size_t c_capacity = 16>
    class lockfree_container {
    public:

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
                        node.full.release();

                        return true;
                    }
                }
            } while (m_empty.acquire());

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