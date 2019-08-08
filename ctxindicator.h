#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <QByteArray>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QThread>

class GTKWorker : public QObject {
  Q_OBJECT

 public slots:
  void init() {
    qDebug() << "start run gtk_main():" << QThread::currentThreadId();
    gtk_main();
  }
};

class CtxIndicator : public QObject {
  Q_OBJECT

 public:
  CtxIndicator(int argc, char *argv[]);

  ~CtxIndicator();

 public:
  void initGTK(int argc, char *argv[]);
  void initCtxIndicator();
  static void activate_action(GtkMenuItem *item, void *p_ctxIndicator);
  static void toggleCtx(GtkMenuItem *item, void *p_ctxIndicator);

  void emitAction(QString actionName);
  void emitCtxIndexChanged(const int &index);
  inline gchar *qstring2charp(const QString &str);
  void addCtxAction(const QString &name);
  void addAction(const QString &name);

 public slots:
  void setCtxLabel(const QString &label);
  void addCtxsToAction(const QStringList &ctxs);
  void updateCtxAction(const int &ctxIndex, const QString &ctxName);

 public:
  QThread gtkWorkerThread;
  GTKWorker gtkWorker;

  GtkWidget *window;
  AppIndicator *indicator;
  GtkWidget *indicatorMenu;
  GtkWidget *ctxMenu;
  GtkWidget *menuItemSWitchContext;
  GtkWidget *lastCtxWidget = nullptr;
  int lastCtxIndex = -1;

 signals:
  void run();
  void actionActive(QString &actionName);
  void ctxIndexChanged(const int &index);
};

#endif  // CONTROLLER_H
