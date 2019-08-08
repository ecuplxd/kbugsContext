#include <ctxindicator.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QObject>
#include "kbugscontext.h"

int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  //    https://doc.qt.io/qt-5/qt.html#ApplicationAttribute-enum
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  QApplication a(argc, argv);

  KBugsContext w;

#if defined(Q_OS_LINUX)
  CtxIndicator ctx(argc, argv);
  QObject::connect(&w, &KBugsContext::showCtxInMenuBar, &ctx,
                   &CtxIndicator::setCtxLabel);
  QObject::connect(&w, &KBugsContext::addCtxs, &ctx,
                   &CtxIndicator::addCtxsToAction);
  QObject::connect(&w, &KBugsContext::showCtxInMenuBar, &ctx,
                   &CtxIndicator::setCtxLabel);
  QObject::connect(&w, &KBugsContext::ctxNameChanged, &ctx,
                   &CtxIndicator::updateCtxAction);

  QObject::connect(&ctx, &CtxIndicator::actionActive, &w,
                   &KBugsContext::doAction);
  QObject::connect(&ctx, SIGNAL(ctxIndexChanged(const int &)), &w,
                   SLOT(switchCtx(const int &)));

#endif

  w.move((a.desktop()->width() - w.width()) / 2,
         (a.desktop()->height() - w.height()) / 2);
  w.show();

  w.emitAddCtxs();

  return a.exec();
}
