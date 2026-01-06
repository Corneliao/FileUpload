#include "TcpManager.h"
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSharedPointer>
#include <QVector>
int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  TcpManager::instance()->create_thread("127.0.0.1", 50060);

  QObject::connect(&app, &QGuiApplication::aboutToQuit, &app,
                   []() { TcpManager::instance()->stop(); });
  QQmlApplicationEngine engine;
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("file_upload", "Main");

  return app.exec();
}
