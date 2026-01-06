#include "TcpManager.h"
#include <QDebug>
#include <QGuiApplication>
TcpManager::TcpManager() {}

TcpManager::~TcpManager() {}

void TcpManager::create_thread(const QString &host, quint16 port) {

  m_work = new FileWork;
  m_thread = new QThread;
  m_work->moveToThread(m_thread);
  connect(m_work, &FileWork::progress, this, &TcpManager::progress,
          Qt::QueuedConnection);
  connect(this, &TcpManager::write, m_work, &FileWork::send_data,
          Qt::QueuedConnection);
  connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater,
          Qt::DirectConnection);
  connect(m_thread, &QThread::finished, m_work, &FileWork::deleteLater,
          Qt::DirectConnection);
  connect(
      m_thread, &QThread::destroyed, m_thread,
      [] { qInfo() << "thread destroyed"; }, Qt::DirectConnection);

  m_thread->start();

  QMetaObject::invokeMethod(m_work, "init", Qt::QueuedConnection,
                            Q_ARG(QString, host), Q_ARG(quint16, port));
}

void TcpManager::sendFile(const QJsonObject &fileData) {
  QMetaObject::invokeMethod(m_work, "sendFile", Qt::QueuedConnection,
                            Q_ARG(QJsonObject, fileData));
}

void TcpManager::stop() {
  if (m_thread->isRunning()) {
    m_thread->quit();
    m_thread->wait();
  }
}