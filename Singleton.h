#ifndef SINGLETON_H
#define SINGLETON_H

#include <QObject>
#include <memory>
#include <mutex>
template <typename T> class Singleton {
  Q_DISABLE_COPY_MOVE(Singleton);

  static std::shared_ptr<T> m_instnace;

public:
  static std::shared_ptr<T> instance() {
    static std::once_flag flag;
    std::call_once(flag, []() { m_instnace = std::shared_ptr<T>(new T); });
    return m_instnace;
  }

protected:
  explicit Singleton() = default;
};

template <typename T> std ::shared_ptr<T> Singleton<T>::m_instnace = nullptr;

#endif // SINGLETON_H
