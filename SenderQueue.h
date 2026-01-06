#ifndef SENDERQUEUE_H
#define SENDERQUEUE_H

#include "Constants.h"
#include "Singleton.h"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

using task = file_processing::io_task::SenderTask;

class SenderQueue : public Singleton<SenderQueue> {
  friend class Singleton<SenderQueue>;
  explicit SenderQueue();

public:
  ~SenderQueue();
  void postTask(std::unique_ptr<task> task_);

private:
  void dealMsg();
  std::mutex mutex_;
  std::thread thread_;
  std::deque<std::unique_ptr<task>> tasks;
  std::condition_variable cond_;
  std::atomic<bool> stop_;
};

#endif // SENDERQUEUE_H
