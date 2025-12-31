#ifndef FILEWORK_H
#define FILEWORK_H

#include "Constants.h"
#include <QJsonObject>
#include <QObject>
#include <QQueue>
#include <QTcpSocket>
#include <QTimer>
#include <atomic>
#include <functional>
#include <unordered_map>

class FileWork : public QObject
{
  Q_OBJECT

public:
  explicit FileWork ();
  ~FileWork ();
  Q_INVOKABLE void init (const QString &host, quint16 port);
  Q_INVOKABLE void sendFile (const QJsonObject &fileData);
signals:
  void send_data (MessageID id, const QByteArray &data);
  void recv_data (MessageReplyID id, const QByteArray &data);
  void progress (const QString &msg_id, qreal progress);
  void statusChanged ();

private:
  void setStatus (bool status);
  void status_slot ();
  void write (MessageID id, const QByteArray &data);
  void read_data ();
  void byte_written (qint64 bytes);
  void init_callback ();
  void handle_callback (MessageReplyID id, const QByteArray &data);
  QTcpSocket *_socket;
  QQueue<QByteArray> _blocks;
  QByteArray cur_block;
  std::atomic<bool> is_sending;
  qint64 written_bytes_;
  QQueue<QJsonObject> _tasks;
  QByteArray _recv_buffer;
  QTimer *timer_;
  QString msg_id_tmp;
  qreal progress_;
  bool is_recv = false;
  std::unordered_map<MessageReplyID, std::function<void (const QByteArray &data)>> _call_backs;
};

#endif // FILEWORK_H
