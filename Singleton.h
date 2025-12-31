#ifndef SINGLETON_H
#define SINGLETON_H

#include <QObject>
template <typename T> class Singleton
{
  Q_DISABLE_COPY_MOVE (Singleton);

public:
  static T &instance ()
  {
    static T t;
    return t;
  }

protected:
  explicit Singleton () = default;
};

#endif // SINGLETON_H
