#include "TcpManager.h"
#include <QDebug>
TcpManager::TcpManager () : _file_work (nullptr), thread_ (nullptr) {}

TcpManager::~TcpManager ()
{
  thread_->requestInterruption ();
  thread_->quit ();
  thread_->wait ();
  thread_->deleteLater ();
  _file_work->deleteLater ();
}

void TcpManager::create_thread (const QString &host, quint16 port)
{
  _file_work = new FileWork;
  thread_ = new QThread;
  _file_work->moveToThread (thread_);
  connect (_file_work, &FileWork::progress, this, &TcpManager::progress, Qt::QueuedConnection);
  thread_->start ();
  QMetaObject::invokeMethod (_file_work, "init", Qt::QueuedConnection, Q_ARG (QString, host),
                             Q_ARG (quint16, port));
}

void TcpManager::sendFile (const QJsonObject &fileData)
{
  if (!_file_work)
    return;
  QMetaObject::invokeMethod (_file_work, "sendFile", Qt::QueuedConnection,
                             Q_ARG (QJsonObject, fileData));
}
