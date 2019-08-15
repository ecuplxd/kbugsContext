#include "ctxindicator.h"
#include <QFile>

void CtxIndicator::activate_action(GtkMenuItem *item, void *p_ctxIndicator) {
  const gchar *name = gtk_menu_item_get_label(item);
  CtxIndicator *ctxIndicator = static_cast<CtxIndicator *>(p_ctxIndicator);
  QString nameNoSpace = QString(QLatin1String(name));
  nameNoSpace = nameNoSpace.remove(QRegExp("\\s"));
  ctxIndicator->emitAction(nameNoSpace);
}

void CtxIndicator::toggleCtx(GtkMenuItem *item, void *p_ctxIndicator) {
  CtxIndicator *ctxIndicator = static_cast<CtxIndicator *>(p_ctxIndicator);
  if (ctxIndicator->NO_EMIT_FLAG) {
    return;
  }
  gint index;

  g_object_get(ctxIndicator->ctxMenu, "active", &index, NULL);

  if (index != ctxIndicator->lastCtxIndex) {
    if (ctxIndicator->lastCtxWidget) {
      gtk_check_menu_item_set_active(
          GTK_CHECK_MENU_ITEM(ctxIndicator->lastCtxWidget), FALSE);
    }
    ctxIndicator->lastCtxWidget =
        gtk_menu_get_active(GTK_MENU(ctxIndicator->ctxMenu));
    ctxIndicator->lastCtxIndex = index;
  } else {
    gboolean checked =
        gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));

    if (checked == 0) {
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    }
    return;
  }

  ctxIndicator->emitCtxIndexChanged(index);
}

CtxIndicator::CtxIndicator(int argc, char *argv[]) {
  initGTK(argc, argv);

  connect(this, &CtxIndicator::run, &gtkWorker, &GTKWorker::init);

  gtkWorker.moveToThread(&gtkWorkerThread);
  gtkWorkerThread.start();

  emit run();
}

CtxIndicator::~CtxIndicator() {
  gtk_main_quit();
  gtkWorkerThread.quit();
  gtkWorkerThread.wait();
  qDebug() << "quit gtk thread";
}

void CtxIndicator::initGTK(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  indicatorMenu = gtk_menu_new();
  ctxMenu = gtk_menu_new();
  menuItemSWitchContext = gtk_menu_item_new_with_label("Switch Context");

  GtkWidget *separator = gtk_separator_menu_item_new();

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItemSWitchContext), ctxMenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(indicatorMenu), menuItemSWitchContext);

  addAction("Import Kubeconfig File");
  addAction("Manage Contexts");
  gtk_menu_shell_append(GTK_MENU_SHELL(indicatorMenu), separator);
  addAction("Quit");

  gtk_widget_show_all(indicatorMenu);

  gchar *name = const_cast<char *>("kbugsCurrentContext");
  gchar *iconPath = getIndicatorIconPath();
  indicator = app_indicator_new_with_path(
      name, iconName, APP_INDICATOR_CATEGORY_OTHER, iconPath);

  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  setIndicatorLabel("");
  app_indicator_set_menu(indicator, GTK_MENU(indicatorMenu));
}

gchar *CtxIndicator::qstring2charp(const QString &str) {
  QByteArray ba = str.toLatin1();
  gchar *ch = ba.data();
  return ch;
}

void CtxIndicator::setIndicatorLabel(const QString &label) {
  QString str = "";
  if (!label.isEmpty()) {
    str = "  " + label;
  }

  // gchar *ch = qstring2charp(str) my crash
  //  why str.toLatin1().data() may crash
  QByteArray ba = str.toLatin1();
  gchar *ch = ba.data();
  app_indicator_set_label(indicator, ch, "");
}

void CtxIndicator::emitAction(QString actionName) {
  emit actionActive(actionName);
}
void CtxIndicator::emitCtxIndexChanged(const int &index) {
  emit ctxIndexChanged(index);
}

void CtxIndicator::addAction(const QString &name) {
  gchar *menuName = qstring2charp(name);
  GtkWidget *menuItem = gtk_menu_item_new_with_label(menuName);
  gtk_menu_shell_append(GTK_MENU_SHELL(indicatorMenu), menuItem);

  g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(activate_action),
                   this);
}

void CtxIndicator::addCtxAction(const QString &name) {
  gchar *menuName = qstring2charp(name);
  GtkWidget *menuItem = gtk_check_menu_item_new_with_label(menuName);

  gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(menuItem), TRUE);
  gtk_menu_shell_append(GTK_MENU_SHELL(ctxMenu), menuItem);
  g_signal_connect(G_OBJECT(menuItem), "toggled", G_CALLBACK(toggleCtx), this);
}

void CtxIndicator::addCtxsToIndicatorMenu(const QStringList &ctxs) {
  for (int i = 0; i < ctxs.size(); ++i) {
    QString ctx = ctxs[i];
    addCtxAction(ctx);
  }
  gtk_widget_show_all(indicatorMenu);
}

void CtxIndicator::updateCtxActionName(const int &ctxIndex,
                                       const QString &ctxName) {
  GtkMenuItem *item = findCtxMenuItem(ctxIndex);
  if (item) {
    gtk_menu_item_set_label(item, qstring2charp(ctxName));
  }
}

GtkMenuItem *CtxIndicator::findCtxMenuItem(const int &index) {
  GtkWidget *menu =
      gtk_menu_item_get_submenu(GTK_MENU_ITEM(menuItemSWitchContext));
  if (GTK_IS_CONTAINER(menu)) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(menu));
    GList *it = NULL;
    int i = 0;
    for (it = children; it; it = it->next) {
      if (i == index) {
        return GTK_MENU_ITEM(it->data);
      }
      i++;
    }
  }
  return nullptr;
}

void CtxIndicator::activeCtx(const int &ctxIndex) {
  GtkMenuItem *item = findCtxMenuItem(ctxIndex);
  if (item) {
    NO_EMIT_FLAG = true;
    if (lastCtxWidget) {
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lastCtxWidget), FALSE);
    }
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);

    lastCtxIndex = ctxIndex;
    lastCtxWidget = GTK_WIDGET(item);
    NO_EMIT_FLAG = false;
  }
}

gchar *CtxIndicator::getIndicatorIconPath() {
  QString tempPath = "";
  if (tempDir.isValid()) {
    tempPath = tempDir.path();
    qDebug() << "temp dir = " << tempPath;
    QString iconNameWithExt = QString(QLatin1String(iconName)) + ".png";
    QFile::copy(":/images/res/images/" + iconNameWithExt,
                tempPath + "/" + iconNameWithExt);
  }

  gchar *path = qstring2charp(tempPath);
  return path;
}
