#pragma once

#include "common.hpp"

namespace Network {

template <typename T> class ThreadSafeQueue {
public:
  ThreadSafeQueue() = default;
  ThreadSafeQueue(const ThreadSafeQueue<T> &) = delete;
  virtual ~ThreadSafeQueue() { clear(); }

public:
  const T &front() {
    std::scoped_lock lock(muxQueue_);
    return deqQueue_.front();
  }

  const T &back() {
    std::scoped_lock lock(muxQueue_);
    return deqQueue_.back();
  }

  T pop_front() {
    std::scoped_lock lock(muxQueue_);
    auto t = std::move(deqQueue_.front());
    deqQueue_.pop_front();
    return t;
  }

  T pop_back() {
    std::scoped_lock lock(muxQueue_);
    auto t = std::move(deqQueue_.back());
    deqQueue_.pop_back();
    return t;
  }

  void push_back(const T &item) {
    std::scoped_lock lock(muxQueue_);
    deqQueue_.emplace_back(std::move(item));

    std::unique_lock<std::mutex> ul(muxBlocking_);
    cvBlocking_.notify_one();
  }

  void push_front(const T &item) {
    std::scoped_lock lock(muxQueue_);
    deqQueue_.emplace_front(std::move(item));

    std::unique_lock<std::mutex> ul(muxBlocking_);
    cvBlocking_.notify_one();
  }

  bool empty() {
    std::scoped_lock lock(muxQueue_);
    return deqQueue_.empty();
  }

  size_t count() {
    std::scoped_lock lock(muxQueue_);
    return deqQueue_.size();
  }

  void clear() {
    std::scoped_lock lock(muxQueue_);
    deqQueue_.clear();
  }

  void wait() {
    while (empty()) {
      std::unique_lock<std::mutex> ul(muxBlocking_);
      cvBlocking_.wait(ul);
    }
  }

private:
  std::mutex muxQueue_;
  std::deque<T> deqQueue_;
  std::condition_variable cvBlocking_;
  std::mutex muxBlocking_;
};

} // namespace Network