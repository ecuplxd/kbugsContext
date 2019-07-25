#include <QApplication>
#include "kbugscontext.h"

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);

  KBugsContext w;

  w.show();

  return a.exec();
}
