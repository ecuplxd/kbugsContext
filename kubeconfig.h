#ifndef CONFIG_H
#define CONFIG_H

#include <QList>
// 必须引入，否则编译报语法错误
#include <QMetaType>
#include <QString>

struct Cluster {
  QString cluster;
  QString certificateAuthorityData;
  QString name;
  QString server;
};

Q_DECLARE_METATYPE(Cluster)

struct Context {
  QString cluster;
  QString user;
  QString name;
  QString nameSpace;
};

Q_DECLARE_METATYPE(Context)

struct User {
  QString name;

  QString clientCertificateData;
  QString clientKeyData;
};

Q_DECLARE_METATYPE(User)

class KubeConfig {
 public:
  KubeConfig();

 public:
  QString apiVersion;
  QString kind;
  QString currentContext;
  QList<Cluster> clusters;
  QList<Context> contexts;
  QList<User> users;
};

Q_DECLARE_METATYPE(KubeConfig)

#endif  // CONFIG_H
