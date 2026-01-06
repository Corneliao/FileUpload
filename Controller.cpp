#include "Controller.h"
#include "TcpManager.h"
#include <QFile>
#include <QFileInfo>
Controller::Controller(QObject *parent) : QObject{parent} {
  connect(TcpManager::instance().get(), &TcpManager::progress, this,
          [this](const QString &msg_id, qreal progress) {
            auto item = progress_items[msg_id];
            if (item) {
              item->setProperty("value", progress);
            }
          });
}

Controller *Controller::instance() {
  static Controller m_instnace;
  return &m_instnace;
}

Controller *Controller::create(QQmlEngine *engine, QJSEngine *) {
  auto obj = instance();
  engine->setObjectOwnership(obj, QQmlEngine::CppOwnership);
  return obj;
}

QJsonObject Controller::parseFile(const QString &path) {
  QFile file(path);
  if (!file.exists()) {
    qInfo() << " file is not exist!";
    return {};
  }

  QFileInfo info(path);
  QJsonObject fileObj;
  QString fileName = info.fileName();
  qint64 fileSize = file.size();
  fileObj["fileName"] = fileName;
  fileObj["filePath"] = info.absoluteFilePath();
  fileObj["fileSize"] = QString::number(fileSize);
  return fileObj;
}

void Controller::sendFile(const QJsonObject &fileData) {
  TcpManager::instance()->sendFile(fileData);
}

QString Controller::generatorUuid() {
  return QUuid::createUuid().toByteArray().toHex();
}

void Controller::insertItem(QString msg_id, QQuickItem *item) {
  if (progress_items.find(msg_id) != progress_items.end()) {
    return;
  }

  progress_items[msg_id] = item;
}
