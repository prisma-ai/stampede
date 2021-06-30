//
// Created by soultoxik on 25.06.2021.
//

#ifndef GRAPH_PROC_SRC_ASYNC_POOL_H_
#define GRAPH_PROC_SRC_ASYNC_POOL_H_

#include <type_traits>
#include <future>
#include <functional>
#include <queue>
#include <iostream>

struct DefaultThreadHooks {
  void preambula() {
    std::cout << "thread created" << std::endl;
  }

  void postmortem() {
    std::cout << "thread destroyed" << std::endl;
  }
};

class PoolBase {
 public:
  template<typename Output>
  std::shared_future<Output> enqueue(std::function<Output(void)> f) {


    std::packaged_task<Output()> task(f);
    auto future = task.get_future();

    {
      std::lock_guard<std::mutex> _(mutex_);
      queue_.push(std::packaged_task<void()>(std::move(task)));
    }
    condition_.notify_one();

    return future;
  }
 protected:

  std::queue<std::packaged_task<void()>> queue_;
  std::mutex mutex_;

  std::condition_variable condition_;
};

template<typename ThreadHooks = DefaultThreadHooks>
class Pool : ThreadHooks, public PoolBase {
 public:
  void start(int N) {
    for(auto i = 0; i < N; ++i) {

      pool_.emplace_back([this]() {
        static_cast<ThreadHooks*>(this)->preambula();

        while(true) {
          std::unique_lock<std::mutex> lock(mutex_);
          condition_.wait(lock, [this] { return !queue_.empty(); });
          auto task = std::move(queue_.front());

          if(task.valid()) {
            queue_.pop();
            lock.unlock();

            task();
          } else {
            break;
          }

        }
        static_cast<ThreadHooks*>(this)->postmortem();


      });
    }
  }

  ~Pool() {
    {
      std::lock_guard<std::mutex> _(mutex_);
      queue_.push({});
    }

    condition_.notify_all();
    for(auto& thread: pool_) {
      thread.join();
    }
    pool_.clear();
  }


 private:
  std::vector<std::thread> pool_;
};
#endif //GRAPH_PROC_SRC_ASYNC_POOL_H_
