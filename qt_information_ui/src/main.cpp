#include <iostream>
#include <QApplication>
#include "mainwindow.h"

using namespace qt_information_ui;

int main(int argc, char** argv)
{
  auto mediaClient = std::make_shared<MediaClient>(
      grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  QApplication app(argc, argv);
  MainWindow w(std::move(mediaClient));
  w.show();
  return app.exec();
}