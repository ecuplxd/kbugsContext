#ifndef KBUGSCONTEXT_H
#define KBUGSCONTEXT_H

#include <QActionGroup>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfoList>
#include <QKeyEvent>
#include <QSettings>
#include <QStandardItem>
#include <QSystemTrayIcon>
#include <QWidget>

#include "include/yaml-cpp/yaml.h"
#include "kubeconfig.h"

namespace Ui {
class KBugsContext;
}

class KBugsContext : public QWidget {
  Q_OBJECT

 public:
  explicit KBugsContext(QWidget* parent = nullptr);
  ~KBugsContext();

 public:
  void initUI();
  void initTray();
  void initActions();
  void addCtxToListWidget(const QFileInfoList& configs);
  void clearCtxConfigForm();
  void addCtxConfigToForm(const QString& path, const Context& ctx);
  static QString toQString(const YAML::Node& node);
  void addCtxModel(const QString& path, const Context& ctx);
  void updateCtxModel(const int& index);
  void updateCtxModel(const QString& newCtxName, const QString& newNameSpace,
                      const QString& newCluster, const QString& newUser);
  void addTrayCtxMenuAction(const QStandardItem& ctxItem);
  Context getCurCtx();
  void saveKubeConfig(const QString& path, const YAML::Node& configNode);
  void saveKubeConfig(const QString& path, const QString& ctxName);
  void selectCtxItemByIndex(const int& index, const bool& clicked = true,
                            const bool& ignoredCheckIndex = false);

  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);

 public:
  QList<Context> parseConfigCtxs(const QString& path);
  KubeConfig parseConfig(const QString& path);
  QFileInfoList listDirConfigFiles(const QString& dir = "");
  int findActionIndex(const QAction& action);

 private:
  void closeEvent(QCloseEvent* event);
  void keyPressEvent(QKeyEvent* event);
  void keyReleaseEvent(QKeyEvent* event);

 private slots:

  void on_revertCtxConfigBtn_clicked();
  void on_applyCtxConfigBtn_clicked();
  void on_changeKubeConfigDirBtn_clicked();
  void on_toggleCtxCheckBox_toggled(bool checked);
  void on_ctxListView_clicked(const QModelIndex& index,
                              const bool& ignoredCheckIndex = false);
  void on_nameLineEdit_textChanged(const QString& arg1);
  void on_clusterCombox_currentIndexChanged(int index);
  void on_nameSpaceLineEdit_textChanged(const QString& arg1);
  void on_clusterCombox_currentIndexChanged(const QString& arg1);
  void on_userComboBox_currentIndexChanged(const QString& arg1);

  void showListViewItemMenu(const QPoint&);
  void switchCtx();
  void switchCtx(const QAction& action);
  void execSwitchCtxCommand(const QString& path);
  void showNormal();
  void trayIsActived(QSystemTrayIcon::ActivationReason reason);
  void toggleRevertAndApply();

  void on_fromServerBtn_clicked();

 private:
  Ui::KBugsContext* ui;

  QString kubeConfigDir = QDir::homePath() + "/.kube/config";

  QStandardItemModel* ctxsModel;
  QMenu* menu;

  QSystemTrayIcon* tray;
  QSystemTrayIcon* ctxTray;
  QMenu* trayMenu;
  QMenu* ctxMenu;
  QAction* importKubeConfigFileAction;
  QAction* manageCtxActions;
  QAction* quitAction;
  QActionGroup* ctxMenuActions;
  QSettings settings;
  QString curConfigPath;
  int curCtxItemIndex = -1;
  int lastActiveCtxIndex = -1;
};

#endif  // KBUGSCONTEXT_H
