#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QJsonObject>
#include <QtTypes>
constexpr int MAX_FILE_LEN = 1024 * 4;
constexpr int HEAD_TOTAL_LEN = 6;
constexpr quint16 HEAD_ID_LEN = 2;
constexpr int HEAD_BODY_LEN = 2;

enum MessageID : qint16
{
  SEND_FILE = 1001,
  SEND_FILE_RSP = 1002,
};

namespace file_processing
{
namespace io_task
{
struct SenderTask
{
  MessageID id;
  QByteArray data;
  SenderTask (MessageID id_, const QByteArray &data_) : id (id_), data (data_) {}
};
}
};

#endif // CONSTANTS_H
