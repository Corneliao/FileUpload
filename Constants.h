#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtTypes>

constexpr int MAX_FILE_LEN = 1024 * 4;
constexpr int HEAD_TOTAL_LEN = 6;
constexpr quint16 HEAD_ID_LEN = 2;
constexpr int HEAD_BODY_LEN = 2;

enum MessageID : qint16
{
  UPLOAD = 1001
};

enum MessageReplyID : qint16
{
  UPLOAD_REPLY = 1002
};

#endif // CONSTANTS_H
