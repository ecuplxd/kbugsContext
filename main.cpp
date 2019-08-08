#include <QApplication>
#include <QDesktopWidget>
#include "kbugscontext.h"

int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  //    https://doc.qt.io/qt-5/qt.html#ApplicationAttribute-enum
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  QApplication a(argc, argv);

  KBugsContext w;

  w.move((a.desktop()->width() - w.width()) / 2,
         (a.desktop()->height() - w.height()) / 2);
  w.show();
  
  return a.exec();
}
