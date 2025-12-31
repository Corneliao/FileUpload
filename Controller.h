#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QJsonObject>
#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>
#include <QUuid>
#include <unordered_map>

class Controller : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON
  Q_DISABLE_COPY_MOVE (Controller);
  explicit Controller (QObject *parent = nullptr);

public:
  static Controller *instance ();
  static Controller *create (QQmlEngine *engine, QJSEngine *);
  Q_INVOKABLE QJsonObject parseFile (const QString &path);
  Q_INVOKABLE void sendFile (const QJsonObject &fileData);
  Q_INVOKABLE QString generatorUuid ();
  Q_INVOKABLE void insertItem (QString msg_id, QQuickItem *item);

private:
  std::unordered_map<QString, QQuickItem *> progress_items;
signals:
};

#endif // CONTROLLER_H
