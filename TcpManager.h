#ifndef TCPMANAGER_H
#define TCPMANAGER_H

#include "FileWork.h"
#include "Singleton.h"
#include <QObject>
#include <QPointer>
#include <QThread>
class TcpManager : public QObject, public Singleton<TcpManager> {
  Q_OBJECT
  friend class Singleton<TcpManager>;
  explicit TcpManager();

public:
  ~TcpManager();
  void create_thread(const QString &host, quint16 port);
  void sendFile(const QJsonObject &fileData);
  void stop();
signals:
  void progress(const QString &msg_id, qreal progress);
  void write(MessageID id, const QByteArray &data);

private:
  QPointer<QThread> m_thread; // QPointer 自动变为 nullptr 当对象被删除
  QPointer<FileWork> m_work;
};

#endif // TCPMANAGER_H
