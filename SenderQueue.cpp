#include "SenderQueue.h"
#include "TcpManager.h"
SenderQueue::SenderQueue() : stop_(false) {
  thread_ = std::thread(&SenderQueue::dealMsg, this);
}

SenderQueue::~SenderQueue() {
  stop_.store(true);
  cond_.notify_one();
  thread_.join();
}

void SenderQueue::postTask(std::unique_ptr<task> task_) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    tasks.push_back(std::move(task_));
  }
  cond_.notify_one();
}

void SenderQueue::dealMsg() {
  while (!stop_.load()) {
    std::unique_ptr<task> task_;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_.wait(lock,
                 [this]() -> bool { return !tasks.empty() || stop_.load(); });

      if (stop_.load()) {
        break;
      }

      task_ = std::move(tasks.front());
      tasks.pop_front();
    }

    if (task_) {
      emit TcpManager::instance() -> write(task_->id, task_->data);
    }
  }
}
