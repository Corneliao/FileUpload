#include "FileWork.h"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <qthread.h>

FileWork::FileWork () : is_sending (false), written_bytes_ (0), timer_ (nullptr), progress_ (0) {}

FileWork::~FileWork ()
{
  if (_socket && _socket->isOpen ())
    {
      qInfo () << "socket closed";
      _socket->close ();
    }
}

void FileWork::init (const QString &host, quint16 port)
{
  _socket = new QTcpSocket (this);
  connect (_socket, &QTcpSocket::connected, this, [] () { qInfo () << "connected"; });
  _socket->connectToHost (host, port);
  if (!_socket->waitForConnected ())
    {

      return;
    }

  init_callback ();
  timer_ = new QTimer (this);
  timer_->setInterval (500);
  connect (timer_, &QTimer::timeout, this, [this] () { emit progress (msg_id_tmp, progress_); });
  connect (_socket, &QTcpSocket::readyRead, this, &FileWork::read_data);
  connect (this, &FileWork::send_data, this, &FileWork::write);
  connect (_socket, &QTcpSocket::bytesWritten, this, &FileWork::byte_written);
  connect (this, &FileWork::recv_data, this, &FileWork::handle_callback);
  connect (this, &FileWork::statusChanged, this, &FileWork::status_slot);
}

void FileWork::sendFile (const QJsonObject &fileData)
{
  if (is_sending.load () || is_recv)
    {
      _tasks.enqueue (fileData);
      return;
    }

  const QString &path = fileData.value ("filePath").toString ();

  QFile file (path);
  if (!file.open (QFile::ReadOnly))
    {
      qWarning () << "file open failed";
      return;
    }

  qint64 origin_pos = file.pos ();

  QCryptographicHash hash (QCryptographicHash::Md5);
  if (!hash.addData (&file))
    {
      qWarning () << "generator md5 failed";
      return;
    }

  QString md5 = hash.result ().toHex ();

  const QString &fileName = fileData.value ("fileName").toString ();
  const QString &msg_id = fileData.value ("msg_id").toString ();
  qint64 fileSize = fileData.value ("fileSize").toString ().toLongLong ();
  int last_seq = (fileSize + MAX_FILE_LEN - 1) / MAX_FILE_LEN;

  QJsonObject jsonBase;
  jsonBase["name"] = fileName;
  jsonBase["total_size"] = fileSize;
  jsonBase["md5"] = md5;
  jsonBase["msg_id"] = msg_id;

  file.seek (origin_pos);

  QByteArray buffer;
  int seq = 0;
  qint64 written_bytes = 0;

  while (!file.atEnd () && !thread ()->isInterruptionRequested ())
    {
      buffer = file.read (MAX_FILE_LEN);
      ++seq;
      written_bytes += buffer.size ();

      QJsonObject jsonObj = jsonBase;
      jsonObj["seq"] = seq;
      jsonObj["last"] = (seq == last_seq) ? true : false;
      jsonObj["data"] = QString (buffer.toBase64 ());
      jsonObj["trans_size"] = written_bytes;

      emit send_data (MessageID::UPLOAD, QJsonDocument (jsonObj).toJson (QJsonDocument::Compact));
    }

  file.close ();
}

void FileWork::setStatus (bool status)
{
  if (is_recv == status)
    return;
  is_recv = status;
  emit statusChanged ();
}

void FileWork::status_slot ()
{
  if (is_recv)
    {
      if (timer_->isActive ())
        {
          timer_->stop ();
        }

      timer_->start ();
    }
  else
    {
      if (timer_->isActive ())
        {
          timer_->stop ();
          emit progress (msg_id_tmp, progress_);
        }
    }
}

void FileWork::write (MessageID id, const QByteArray &data)
{
  quint16 msg_id = static_cast<quint16> (id);

  quint32 body_len = static_cast<quint32> (data.length ());

  QByteArray block;

  QDataStream stream (&block, QIODevice::WriteOnly);
  stream.setByteOrder (QDataStream::BigEndian);

  stream << msg_id << body_len;

  block.append (data);

  if (is_sending)
    {
      _blocks.append (block);
      return;
    }

  cur_block = block;
  written_bytes_ = 0;
  is_sending = true;

  _socket->write (block);
}

void FileWork::read_data ()
{
  _recv_buffer.append (_socket->readAll ());

  while (_recv_buffer.size () > HEAD_TOTAL_LEN)
    {
      QByteArray head_data = _recv_buffer.left (HEAD_TOTAL_LEN);

      QDataStream stream (&head_data, QIODevice::ReadOnly);
      stream.setByteOrder (QDataStream::BigEndian);

      quint16 msg_id = 0;
      quint32 body_len = 0;

      stream >> msg_id >> body_len;

      if (_recv_buffer.size () >= HEAD_TOTAL_LEN + body_len)
        {
          QByteArray body_data = _recv_buffer.mid (HEAD_TOTAL_LEN, body_len);
          _recv_buffer = _recv_buffer.mid (HEAD_TOTAL_LEN + body_len);
          emit recv_data (static_cast<MessageReplyID> (msg_id), body_data);
        }
    }
}

void FileWork::byte_written (qint64 bytes)
{
  written_bytes_ += bytes;
  if (written_bytes_ < cur_block.size ())
    {
      QByteArray buffer_temp = cur_block.mid (written_bytes_);
      _socket->write (buffer_temp);
      return;
    }

  if (_blocks.empty ())
    {
      is_sending = false;
      cur_block.clear ();
      written_bytes_ = 0;
      return;
    }

  cur_block = _blocks.dequeue ();
  written_bytes_ = 0;
  _socket->write (cur_block);
}

void FileWork::init_callback ()
{
  _call_backs[MessageReplyID::UPLOAD_REPLY] = [this] (const QByteArray &data)
    {
      QJsonDocument doc = QJsonDocument::fromJson (data);
      const QJsonObject &root = doc.object ();
      qreal process = root.value ("process").toDouble ();
      QString msg_id = root.value ("msg_id").toString ();
      msg_id_tmp = msg_id;
      progress_ = process;
      setStatus (true);
      if (progress_ >= 100)
        {
          setStatus (false);
          if (!_tasks.isEmpty ())
            {
              sendFile (_tasks.dequeue ());
            }
        }
    };
}

void FileWork::handle_callback (MessageReplyID id, const QByteArray &data)
{
  if (_call_backs.find (id) == _call_backs.end ())
    {
      return;
    }

  _call_backs[id](data);
}
