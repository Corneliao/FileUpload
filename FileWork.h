#ifndef FILEWORK_H
#define FILEWORK_H

#include "Constants.h"
#include <QFuture>
#include <QGuiApplication>
#include <QJsonObject>
#include <QObject>
#include <QQueue>
#include <QTcpSocket>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <unordered_map>

class FileWork : public QObject {
  Q_OBJECT

public:
  explicit FileWork();
  ~FileWork() override;
  Q_INVOKABLE void init(const QString &host, quint16 port);
  Q_INVOKABLE void sendFile(const QJsonObject &fileData);

signals:
  void send_data(MessageID id, const QByteArray &data);
  void recv_data(MessageID id, const QByteArray &data);
  void progress(const QString &msg_id, qreal progress);
  void statusChanged();

private:
  void init_inside();
  void setStatus(bool status);
  void status_slot();
  void write(MessageID id, const QByteArray &data);
  void read_data();
  void byte_written(qint64 bytes);
  void init_callback();
  void handle_callback(MessageID id, const QByteArray &data);
  void deal_recv_msg();
  void postRecvTask(std::unique_ptr<file_processing::io_task::SenderTask> task);

  QTcpSocket *_socket;
  QQueue<QByteArray> _blocks;
  QByteArray cur_block;
  std::atomic<bool> is_sending;
  qint64 written_bytes_;
  QQueue<QJsonObject> _tasks;
  QByteArray _recv_buffer;
  QTimer *timer_ = nullptr;
  QString msg_id_tmp;
  qreal progress_;
  bool is_recv = false;
  std::unordered_map<MessageID, std::function<void(const QByteArray &data)>>
      _call_backs;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::atomic<bool> stop_;
  std::deque<std::unique_ptr<file_processing::io_task::SenderTask>> _recv_tasks;
  std::thread thread_;
};

#endif // FILEWORK_H
