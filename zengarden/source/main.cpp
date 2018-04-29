#include "zengarden.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

#ifdef QT_STATIC_BUILD
#include <QtCore/QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  app.setStyle(QStyleFactory::create("fusion"));

  QPalette palette;
  palette.setColor(QPalette::Window, QColor(53, 53, 53));
  palette.setColor(QPalette::WindowText, Qt::lightGray);
  palette.setColor(QPalette::Base, QColor(15, 15, 15));
  palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  palette.setColor(QPalette::ToolTipBase, Qt::white);
  palette.setColor(QPalette::ToolTipText, Qt::white);
  palette.setColor(QPalette::Text, Qt::lightGray);
  palette.setColor(QPalette::Button, QColor(53, 53, 53));
  palette.setColor(QPalette::ButtonText, Qt::lightGray);
  palette.setColor(QPalette::BrightText, Qt::red);

  palette.setColor(QPalette::Highlight, QColor(120, 10, 120).lighter());
  palette.setColor(QPalette::HighlightedText, Qt::black);
  app.setPalette(palette);

  ZenGarden mainWindow;
  mainWindow.show();
  return app.exec();
}
