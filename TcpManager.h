#ifndef TCPMANAGER_H
#define TCPMANAGER_H

#include "FileWork.h"
#include "Singleton.h"
#include <QObject>
#include <QThread>
class TcpManager : public QObject, public Singleton<TcpManager>
{
  friend class Singleton<TcpManager>;
  Q_OBJECT
  explicit TcpManager ();

public:
  ~TcpManager ();
  void create_thread (const QString &host, quint16 port);
  void sendFile (const QJsonObject &fileData);
signals:
  void progress (const QString &msg_id, qreal progress);

private:
  FileWork *_file_work;
  QThread *thread_;
};

#endif // TCPMANAGER_H
