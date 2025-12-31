#include "TcpManager.h"
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
int main (int argc, char *argv[])
{
  QGuiApplication app (argc, argv);

  TcpManager::instance ().create_thread ("127.0.0.1", 8888);

  qInfo () << "主线程" << QThread::currentThreadId ();
  QQmlApplicationEngine engine;
  QObject::connect (
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      [] () { QCoreApplication::exit (-1); }, Qt::QueuedConnection);
  engine.loadFromModule ("file_upload", "Main");

  return app.exec ();
}
