//
//  test_hash_set.cpp
//  lockfree_container
//
//  Created by Dmitrii Torkhov <dmitriitorkhov@gmail.com> on 08.09.2021.
//  Copyright © 2021 Dmitrii Torkhov. All rights reserved.
//

#include <memory>
#include <thread>
#include <unordered_set>
#include <vector>

#include <lockfree_container/lockfree_container.h>

int main() {
    srand(time(nullptr)); //NOLINT

    //

    const auto thread_count = 10;
    const auto iteration_count = 1000;

    std::vector<char> data{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                           'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    //

    oo::lockfree_container<char, 32> set;

    //

    const auto thread_fn = [&]() {
        for (size_t i = 0; i < iteration_count; ++i) {
            const auto c = data[rand() % 26]; //NOLINT
            if (rand() % 2 == 0) { //NOLINT
                set.insert(c);
            } else {
                set.erase(c);
            }
        }
    };

    std::vector<std::shared_ptr<std::thread>> threads;
    for (size_t i = 0; i < thread_count; ++i) {
        threads.push_back(std::make_shared<std::thread>(thread_fn));
    }

    //

    for (const auto &thread: threads) {
        thread->join();
    }

    //

    std::unordered_set<char> original{data.begin(), data.end()};
    for (auto &c: set) {
        original.insert(c);
    }

    return original.size() == data.size() ? 0 : 1;
}