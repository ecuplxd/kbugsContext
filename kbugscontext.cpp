#include "kbugscontext.h"

#include <QAbstractListModel>
#include <QAction>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QUrl>

#include <algorithm>
#include <iostream>
#include <string>
#include "loadconfigfromserverdialog.h"
#include "ui_kbugscontext.h"

using namespace std;

KBugsContext::KBugsContext(QWidget* parent)
    : QWidget(parent), ui(new Ui::KBugsContext) {
  ui->setupUi(this);
  restoreSettings();

  initTray();
  initUI();
  initListViewItemActions();
}

KBugsContext::~KBugsContext() {
  storeSettings();
  delete ui; 
}

void KBugsContext::storeSettings() {
  settings.beginGroup("solodevice_mainwindow");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("kubeConfigDir", kubeConfigDir);
  settings.endGroup();
}

void KBugsContext::restoreSettings(bool showWindow) {
  settings.beginGroup("solodevice_mainwindow");
  kubeConfigDir = settings.value("kubeConfigDir").toString();
  settings.endGroup();
  if (showWindow) {
    show();
    restoreGeometry(settings.value("geometry").toByteArray());
  }
}

void KBugsContext::initTray() {
  QIcon icon(":/images/res/images/tray.ico");

  tray = new QSystemTrayIcon(this);
  tray->setIcon(icon);
  //  not work
  tray->setToolTip(tr("KBugsContext"));

  importKubeConfigFileAction = new QAction(tr("Import Kubeconfig File"), this);
  manageCtxActions = new QAction(tr("Manage Contexts"), this);
  quitAction = new QAction(tr("Quit"), this);

  connect(importKubeConfigFileAction, SIGNAL(triggered()), this,
          SLOT(on_changeKubeConfigDirBtn_clicked()));
  connect(manageCtxActions, SIGNAL(triggered()), this, SLOT(showNormal()));
  connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

  trayMenu = new QMenu(this);
  ctxMenu = new QMenu(trayMenu);
  ctxMenuActions = new QActionGroup(ctxMenu);

  trayMenu->addMenu(ctxMenu);

  ctxMenu->setTitle(tr("SWitch Context"));

  // trayMenu->addAction(importKubeConfigFileAction);
  trayMenu->addAction(manageCtxActions);
  trayMenu->addSeparator();
  trayMenu->addAction(quitAction);

  ctxTray = new QSystemTrayIcon(this);
  ctxTray->hide();

  tray->setContextMenu(trayMenu);
  tray->show();

  connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
          SLOT(trayIsActived(QSystemTrayIcon::ActivationReason)));
}

QFileInfoList KBugsContext::listDirConfigFiles(const QString& dir) {
  QStringList filter;
  QString temp = dir.isEmpty() ? kubeConfigDir : dir;
  QDir dir_(temp);
  filter << "*.config";

  return dir_.entryInfoList(filter, QDir::Files);
}

void KBugsContext::initUI() {
  ctxsModel = new QStandardItemModel;
  ui->kubeConfigPathLineEdit->setText(kubeConfigDir);
  auto configs = listDirConfigFiles();
  addCtxToListWidget(configs);
  setAcceptDrops(true);

  //  ui->ctxListView->setAcceptDrops(true);
  ui->ctxListView->updatesEnabled();
  ui->ctxListView->setModel(ctxsModel);
  selectCtxItemByIndex(0);
}

void KBugsContext::initListViewItemActions() {
  ui->ctxListView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->ctxListView, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(showListViewItemMenu(const QPoint&)));
}

void KBugsContext::showListViewItemMenu(const QPoint&) {
  if (ui->ctxListView->selectionModel()->selectedIndexes().isEmpty()) {
    return;
  }

  const QModelIndex& index = ui->ctxListView->currentIndex();
  on_ctxListView_clicked(index);

  menu = new QMenu(ui->ctxListView);
  QAction* switchToThisCtx = menu->addAction(tr("Switch to this context"));
  connect(switchToThisCtx, SIGNAL(triggered(bool)), this, SLOT(switchCtx()));

  menu->exec(QCursor::pos());
}

void KBugsContext::addCtxToListWidget(const QFileInfoList& configs) {
  foreach (auto config, configs) {
    QString name = config.fileName();
    QString path = config.filePath();
    auto ctxs = parseConfigCtxs(path);
    foreach (const Context& ctx, ctxs) { addCtxModel(path, ctx); }
  }
}

void KBugsContext::addCtxModel(const QString& path, const Context& ctx) {
  for (int i = 0; i < ctxsModel->rowCount(); ++i) {
    auto ctxModel = ctxsModel->item(i);
    if (ctxModel->text() == ctx.name && ctxModel->toolTip() == path) {
      qDebug() << path << ctx.name << "配置已存在";
      return;
    }
  }
  QStandardItem* ctxtItem = new QStandardItem;

  ctxtItem->setText(ctx.name);
  ctxtItem->setToolTip(path);
  ctxtItem->setData(QVariant::fromValue(ctx));
  ctxsModel->appendRow(ctxtItem);
  addTrayCtxMenuAction(*ctxtItem);
}

void KBugsContext::addTrayCtxMenuAction(const QStandardItem& ctxItem) {
  QAction* ctxAction = ctxMenuActions->addAction(ctxItem.text());

  //    使用代码创建的 QAction
  //    默认是没有 setCheckable 的，因此在创建后要把所有的 QAction
  //    setCheckable，否则 setChecked 无效。
  ctxAction->setCheckable(true);

  ctxAction->setToolTip(ctxItem.toolTip());
  ctxAction->setData(ctxItem.toolTip());
  connect(ctxAction, &QAction::triggered, this,
          [ctxAction, this]() { this->switchCtx(*ctxAction); });
  ctxMenu->addAction(ctxAction);
}

void KBugsContext::on_revertCtxConfigBtn_clicked() {
  Context ctx = getCurCtx();

  ui->nameLineEdit->setText(ctx.name);
  ui->clusterCombox->setCurrentText(ctx.cluster);
  ui->nameSpaceLineEdit->setText(ctx.nameSpace);
  ui->userComboBox->setCurrentText(ctx.user);
}

void KBugsContext::on_changeKubeConfigDirBtn_clicked() {
  QString dir = "./";

  if (!kubeConfigDir.isEmpty()) {
    dir = kubeConfigDir;
  }
  QString directory = QFileDialog::getExistingDirectory(
      this, tr("Choose KubeConfig Dir"), kubeConfigDir);

  if (!directory.isEmpty()) {
    kubeConfigDir = directory;
    ui->kubeConfigPathLineEdit->setText(kubeConfigDir);
    auto configs = listDirConfigFiles();
    addCtxToListWidget(configs);
  }
}

void KBugsContext::on_toggleCtxCheckBox_toggled(bool checked) {
  if (checked) {
    ctxTray->show();
  } else {
    ctxTray->hide();
  }
}

void KBugsContext::switchCtx(const QAction& action) {
  QString path = action.toolTip();

  auto actions = ctxMenuActions->actions();
  int index = findActionIndex(action);

  if (index != -1) {
    curConfigPath = path;
    saveKubeConfig(path, action.text());
    execSwitchCtxCommand(path);
    updateCtxModel(index);
    selectCtxItemByIndex(index);
  }
}

int KBugsContext::findActionIndex(const QAction& action) {
  auto actions = ctxMenuActions->actions();
  //  TODO: use qFind?
  for (int j = 0; j < actions.size(); ++j) {
    auto c = actions.at(j);
    if (c->text() == action.text() && c->toolTip() == action.toolTip()) {
      return j;
    }
  }
  return -1;
}

void KBugsContext::selectCtxItemByIndex(const int& index, const bool& clicked,
                                        const bool& ignoredCheckIndex) {
  if (index >= ctxsModel->rowCount()) {
    return;
  }
  auto indexModel = ctxsModel->index(index, 0);
  ui->ctxListView->setCurrentIndex(indexModel);
  if (clicked) {
    on_ctxListView_clicked(indexModel, ignoredCheckIndex);
  }
}

void KBugsContext::switchCtx() {
  Context context = getCurCtx();

  ctxMenuActions->actions()[curCtxItemIndex]->setChecked(true);
  saveKubeConfig(curConfigPath, context.name);
  execSwitchCtxCommand(curConfigPath);
  updateCtxModel(curCtxItemIndex);
  setWindowTitle("KBugsContext (" + context.name + ")");
}

void KBugsContext::execSwitchCtxCommand(const QString& path) {
  //    TODO: 采用改环境变量的方式？
  //  QString command = "export KUBECONFIG=" + path;
  QString command = "KUBECONFIG=" + path +
                    " kubectl config view --raw> "
                    "$HOME/.kube/config";
  system(command.toStdString().c_str());
}

void KBugsContext::saveKubeConfig(const QString& path, const QString& ctxName) {
  YAML::Node configNode = YAML::LoadFile(path.toStdString());
  configNode["current-context"] = ctxName.toStdString();
  saveKubeConfig(path, configNode);
  // TODO: tray 实现 text
  //  ctxTray->setIcon(KBugsContext::text2Pixmap(ctxName));
}

void KBugsContext::on_ctxListView_clicked(const QModelIndex& index,
                                          const bool& ignoredCheckIndex) {
  if (index.row() == curCtxItemIndex && !ignoredCheckIndex) {
    return;
  }

  curCtxItemIndex = index.row();

  QStandardItem* ctxItem = ctxsModel->itemFromIndex(index);
  QString path = ctxItem->toolTip();
  Context ctx = ctxItem->data().value<Context>();

  curConfigPath = path;
  addCtxConfigToForm(path, ctx);
}

void KBugsContext::updateCtxModel(const QString& newCtxName,
                                  const QString& newNameSpace,
                                  const QString& newCluster,
                                  const QString& newUser) {
  auto index = ui->ctxListView->currentIndex();
  QStandardItem* ctxItem = ctxsModel->itemFromIndex(index);
  // 优化
  Context ctx = ctxItem->data().value<Context>();

  ctx.name = newCtxName;
  ctx.nameSpace = newNameSpace;
  ctx.cluster = newCluster;
  ctx.user = newUser;

  ctxItem->setText(newCtxName);
  ctxItem->setData(QVariant::fromValue(ctx));

  //  无需重新设置，自动感应数据变化
  //  ctxsModel->setItem(index.row(), 0, ctxtItem);
  ctxMenuActions->actions()[index.row()]->setText(newCtxName);
  toggleRevertAndApply();
}

void KBugsContext::updateCtxModel(const int& index) {
  if (lastActiveCtxIndex > -1) {
    QStandardItem* lastCtxItem = ctxsModel->item(lastActiveCtxIndex);
    Context lastCtx = lastCtxItem->data().value<Context>();
    lastCtxItem->setText(lastCtx.name);
  }
  QStandardItem* ctxItem = ctxsModel->item(index);
  Context ctx = ctxItem->data().value<Context>();
  ctxItem->setText(ctx.name + " " + tr("(Current Context)"));
  lastActiveCtxIndex = index;
}

QList<Context> KBugsContext::parseConfigCtxs(const QString& path) {
  YAML::Node config;
  QList<Context> ctxs;
  try {
    config = YAML::LoadFile(path.toStdString());
  } catch (YAML::ParserException) {
    QMessageBox::critical(this, tr("Parse Error"),
                          path + " is a invalid kubeconfig file");
    return ctxs;
  }

  const YAML::Node& ctxNodes = config["contexts"];

  for (unsigned i = 0; i < ctxNodes.size(); ++i) {
    Context c;
    auto ctxNode = ctxNodes[i];

    QString name = KBugsContext::toQString(ctxNode["name"]);
    QString nameSpace =
        KBugsContext::toQString(ctxNode["context"]["namespace"]);
    QString clusterName =
        KBugsContext::toQString(ctxNode["context"]["cluster"]);
    QString user = KBugsContext::toQString(ctxNode["context"]["user"]);

    c.name = name;
    c.cluster = clusterName;
    c.user = user;
    c.nameSpace = nameSpace;
    ctxs.append(c);
  }
  return ctxs;
}

KubeConfig KBugsContext::parseConfig(const QString& path) {
  KubeConfig kubeConfig;

  YAML::Node config = YAML::LoadFile(path.toStdString());

  auto apiVersion = KBugsContext::toQString(config["apiVersion"]);
  auto kind = KBugsContext::toQString(config["kind"]);
  auto currentCtx = KBugsContext::toQString(config["current-context"]);

  kubeConfig.apiVersion = apiVersion;
  kubeConfig.kind = kind;
  kubeConfig.currentContext = currentCtx;

  const YAML::Node& clusters = config["clusters"];
  const YAML::Node& contexts = config["contexts"];
  const YAML::Node& users = config["users"];

  for (unsigned i = 0; i < clusters.size(); ++i) {
    Cluster c;
    auto cluster = clusters[i];

    QString name = KBugsContext::toQString(cluster["name"]);
    QString server = KBugsContext::toQString(cluster["cluster"]["server"]);
    QString certificateAuthorityData = KBugsContext::toQString(
        cluster["cluster"]["certificate-authority-data"]);

    c.name = name;
    c.server = server;
    c.certificateAuthorityData = certificateAuthorityData;
    kubeConfig.clusters.append(c);
  }

  for (unsigned i = 0; i < contexts.size(); ++i) {
    Context c;
    auto context = contexts[i];

    QString name = KBugsContext::toQString(context["name"]);
    QString nameSpace =
        KBugsContext::toQString(context["context"]["namespace"]);
    QString clusterName =
        KBugsContext::toQString(context["context"]["cluster"]);
    QString user = KBugsContext::toQString(context["context"]["user"]);

    c.name = name;
    c.cluster = clusterName;
    c.user = user;
    c.nameSpace = nameSpace;
    kubeConfig.contexts.append(c);
  }

  for (unsigned i = 0; i < users.size(); ++i) {
    User u;
    auto user = users[i];

    QString name = KBugsContext::toQString(user["name"]);
    QString clientKeyData =
        KBugsContext::toQString(user["user"]["client-key-data"]);
    QString clientCertificateData =
        KBugsContext::toQString(user["user"]["client-certificate-data"]);

    u.name = name;
    u.clientKeyData = clientKeyData;
    u.clientCertificateData = clientCertificateData;
    kubeConfig.users.append(u);
  }
  return kubeConfig;
}

void KBugsContext::clearCtxConfigForm() {
  ui->clusterCombox->clear();
  ui->userComboBox->clear();
}

void KBugsContext::addCtxConfigToForm(const QString& path, const Context& ctx) {
  clearCtxConfigForm();
  KubeConfig kubeConfig = parseConfig(path);
  auto ctxs = kubeConfig.contexts;

  QString name = ctx.name;
  QString nameSpace = ctx.nameSpace;
  QString cluster = ctx.cluster;
  QString user = ctx.user;

  foreach (auto cluster, kubeConfig.clusters) {
    ui->clusterCombox->addItem(cluster.name, QVariant::fromValue(cluster));
  }
  foreach (auto user, kubeConfig.users) {
    ui->userComboBox->addItem(user.name, QVariant::fromValue(user));
  }

  ui->nameLineEdit->setText(name);
  ui->nameSpaceLineEdit->setText(nameSpace);
  ui->clusterCombox->setCurrentText(cluster);
  ui->userComboBox->setCurrentText(user);
}

QString KBugsContext::toQString(const YAML::Node& node) {
  return QString::fromStdString(node.as<string>(""));
}

void KBugsContext::on_clusterCombox_currentIndexChanged(int index) {
  QVariant data = ui->clusterCombox->itemData(index);
  Cluster cluster = data.value<Cluster>();
  QString server = cluster.server;

  ui->clusterServerLabel->setText("Server:" + server);
}

void KBugsContext::closeEvent(QCloseEvent* event) {
  storeSettings();

  hide();
#ifdef QT_NO_DEBUG
  event->ignore();
#else   // ifdef QT_NO_DEBUG
  event->accept();
#endif  // ifdef QT_NO_DEBUG
}

void KBugsContext::showNormal() { restoreSettings(true); }

void KBugsContext::trayIsActived(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
    case QSystemTrayIcon::Trigger: {
      showNormal();
      break;
    }
    case QSystemTrayIcon::DoubleClick: {
      showNormal();
      break;
    }
    default:
      break;
  }
}

void KBugsContext::on_nameLineEdit_textChanged(const QString& arg1) {
  toggleRevertAndApply();
}

void KBugsContext::on_nameSpaceLineEdit_textChanged(const QString& arg1) {
  toggleRevertAndApply();
}

void KBugsContext::on_clusterCombox_currentIndexChanged(const QString& arg1) {
  toggleRevertAndApply();
}

void KBugsContext::on_userComboBox_currentIndexChanged(const QString& arg1) {
  toggleRevertAndApply();
}

Context KBugsContext::getCurCtx() {
  QModelIndex index = ui->ctxListView->currentIndex();
  auto data = ctxsModel->itemFromIndex(index)->data();
  Context ctx = data.value<Context>();
  return ctx;
}

void KBugsContext::toggleRevertAndApply() {
  if (ctxIsEmpty()) {
    return;
  }

  Context ctx = getCurCtx();
  bool changed = false;

  if (ui->nameLineEdit->text() != ctx.name) {
    changed = true;
  } else if (ui->nameSpaceLineEdit->text() != ctx.nameSpace) {
    changed = true;
  } else if (ui->clusterCombox->currentText() != ctx.cluster) {
    changed = true;
  } else if (ui->userComboBox->currentText() != ctx.user) {
    changed = true;
  }

  ui->revertCtxConfigBtn->setDisabled(!changed);
  ui->applyCtxConfigBtn->setDisabled(!changed);
}

void KBugsContext::on_applyCtxConfigBtn_clicked() {
  Context ctx = getCurCtx();
  YAML::Node configNode = YAML::LoadFile(curConfigPath.toStdString());
  const YAML::Node& ctxNodes = configNode["contexts"];

  auto newCtxName = ui->nameLineEdit->text();
  auto newNameSpace = ui->nameSpaceLineEdit->text();
  auto newCluster = ui->clusterCombox->currentText();
  auto newUser = ui->userComboBox->currentText();

  for (unsigned i = 0; i < ctxNodes.size(); ++i) {
    YAML::Node ctxNode = ctxNodes[i];
    QString name = KBugsContext::toQString(ctxNode["name"]);

    //    if (name == newCtxName) {
    //      ui->nameLineEdit->selectAll();
    //      ui->nameLineEdit->setFocus();
    //      QMessageBox::critical(this, tr("Saved Error"),
    //                            QString("Context name  %1 exits").arg(name));
    //      return;
    //    }

    if (name == ctx.name) {
      configNode["contexts"][i]["name"] = newCtxName.toStdString();
      configNode["contexts"][i]["context"]["namespace"] =
          newNameSpace.toStdString();
      configNode["contexts"][i]["context"]["cluster"] =
          newCluster.toStdString();
      configNode["contexts"][i]["context"]["user"] = newUser.toStdString();

      auto curCtx = KBugsContext::toQString(configNode["current-context"]);
      if (curCtx == ctx.name) {
        configNode["current-context"] = newCtxName.toStdString();
      }
      break;
    }
  }
  saveKubeConfig(curConfigPath, configNode);
  updateCtxModel(newCtxName, newNameSpace, newCluster, newUser);
}

void KBugsContext::saveKubeConfig(const QString& path,
                                  const YAML::Node& configNode) {
  auto configData = YAML::Dump(configNode);
  QFile configFile(path);

  if (!configFile.open(QIODevice::WriteOnly)) {
    qDebug() << "could't open kubeconfig file";
  }
  configFile.write(configData.c_str());
  configFile.close();
}

void KBugsContext::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void KBugsContext::dropEvent(QDropEvent* event) {
  QList<QUrl> urls = event->mimeData()->urls();
  if (urls.isEmpty()) {
    return;
  }

  QFileInfoList configs;
  foreach (auto url, urls) {
    QString path = url.path();
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
      addCtxToListWidget(listDirConfigFiles(path));
    } else {
      configs << path;
    }
  }

  addCtxToListWidget(configs);
}

void KBugsContext::on_fromServerBtn_clicked() {
  LoadConfigFromServerDialog* dlg = new LoadConfigFromServerDialog(this);
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  dlg->setWindowModality(Qt::ApplicationModal);
  int ret = dlg->exec();
  if (ret == QDialog::Accepted) {
    qDebug() << "你点击了OK按钮";
  } else if (ret == QDialog::Rejected) {
    qDebug() << "你点击了Cancle按钮";
  }
}

bool KBugsContext::ctxIsEmpty() { return ctxsModel->rowCount() == 0; }

void KBugsContext::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Tab &&
      event->modifiers() == Qt::ControlModifier) {
    if (ctxIsEmpty()) {
      return;
    }
    curCtxItemIndex += 1;
    if (curCtxItemIndex == ctxsModel->rowCount()) {
      curCtxItemIndex = 0;
    }
    selectCtxItemByIndex(curCtxItemIndex, false);
  }
}

void KBugsContext::keyReleaseEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Control) {
    if (ctxIsEmpty()) {
      return;
    }
    selectCtxItemByIndex(curCtxItemIndex, true, true);
    switchCtx();
  }
}
