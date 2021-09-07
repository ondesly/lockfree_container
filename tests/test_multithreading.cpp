//
//  test_multithreading.cpp
//  lockfree_container
//
//  Created by Dmitrii Torkhov <dmitriitorkhov@gmail.com> on 07.09.2021.
//  Copyright © 2021 Dmitrii Torkhov. All rights reserved.
//

#include <iostream>
#include <numeric>
#include <random>
#include <thread>

#include <lockfree_container/lockfree_container.h>

namespace {

    const auto c_iteration_count = 100;

}

int main() {
    for (size_t j = 0; j < c_iteration_count; ++j) {
        const auto data_len = 5 + rand() % 96;  // NOLINT
        const auto producer_count = 1 + rand() % 10;  // NOLINT
        const auto consumer_count = 1 + rand() % 5;  // NOLINT

        std::cout << j << ": len " << data_len << ", producers " << producer_count << ", consumers " << consumer_count
                  << std::endl;

        //

        std::vector<int> data(data_len);

        std::random_device rd;
        std::default_random_engine engine(rd());
        std::generate(data.begin(), data.end(), std::ref(engine));

        //

        oo::lockfree_container<int, 1000> queue;

        //

        const auto producer_fn = [&]() {
            for (size_t i = 0; i < data_len; ++i) {
                const auto value = data[i];
                queue.push(value);
            }
        };

        std::vector<std::shared_ptr<std::thread>> producers;
        for (size_t i = 0; i < producer_count; ++i) {
            producers.push_back(std::make_shared<std::thread>(producer_fn));
        }

        //

        std::vector<int> results[consumer_count];

        bool is_done = false;
        const auto consumer_fn = [&](size_t i) {
            int num;
            do {
                while (queue.pop(num)) {
                    results[i].push_back(num);
                }
            } while (!is_done);
        };

        std::vector<std::shared_ptr<std::thread>> consumers;
        for (size_t i = 0; i < consumer_count; ++i) {
            consumers.push_back(std::make_shared<std::thread>(consumer_fn, i));
        }

        //

        for (const auto &producer: producers) {
            producer->join();
        }

        is_done = true;

        //

        for (const auto &consumer: consumers) {
            consumer->join();
        }

        //

        std::vector<int> result;
        for (const auto &r: results) {
            result.insert(result.end(), r.begin(), r.end());
        }

        const auto data_sum = std::accumulate(data.begin(), data.end(), 0LL);
        const auto result_sum = std::accumulate(result.begin(), result.end(), 0LL);

        //

        if (result_sum != data_sum * producer_count) {
            return 1;
        }
    }

    return 0;
}