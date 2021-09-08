//
//  test_blocking_wait.cpp
//  lockfree_container
//
//  Created by Dmitrii Torkhov <dmitriitorkhov@gmail.com> on 08.09.2021.
//  Copyright © 2021 Dmitrii Torkhov. All rights reserved.
//

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <numeric>
#include <random>
#include <thread>

#include <lockfree_container/lockfree_container.h>

namespace {

    const auto c_iteration_count = 10000;

}

int main() {
    for (size_t j = 0; j < c_iteration_count; ++j) {
        const auto data_len = 22;
        const auto producer_count = 4;
        const auto consumer_count = 4;

        std::cout << j << std::endl;

        //

        std::vector<int> data(data_len);

        std::random_device rd;
        std::default_random_engine engine(rd());
        std::generate(data.begin(), data.end(), std::ref(engine));

        //

        oo::lockfree_container<int, 100> queue;

        std::mutex mutex;
        std::condition_variable condition;

        //

        const auto producer_fn = [&]() {
            for (size_t i = 0; i < data_len; ++i) {
                queue.push(data[i]);
                condition.notify_all();

                if (i % 10 == 0) {
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(10us);
                }
            }
        };

        std::vector<std::shared_ptr<std::thread>> producers;
        for (size_t i = 0; i < producer_count; ++i) {
            producers.push_back(std::make_shared<std::thread>(producer_fn));
        }

        //

        bool is_done = false;

        std::vector<int> results[consumer_count];

        const auto consumer_fn = [&](size_t i) {
            int num;
            while (true) {
                if (queue.pop(num)) {
                    results[i].push_back(num);
                } else {
                    std::unique_lock<std::mutex> lock(mutex);
                    if (is_done) {
                        break;
                    } else {
                        using namespace std::chrono_literals;
                        condition.wait_for(lock, 100ms);
                    }
                }
            }
        };

        std::vector<std::shared_ptr<std::thread>> consumers;
        for (size_t i = 0; i < consumer_count; ++i) {
            consumers.push_back(std::make_shared<std::thread>(consumer_fn, i));
        }

        //

        for (const auto &producer: producers) {
            producer->join();
        }

        {
            std::lock_guard<std::mutex> lock(mutex);

            is_done = true;
            condition.notify_all();
        }

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