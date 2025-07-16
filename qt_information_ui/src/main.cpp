#include <iostream>
#include <QApplication>
#include "mainwindow.h"

using namespace qt_information_ui;

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  return app.exec();
}