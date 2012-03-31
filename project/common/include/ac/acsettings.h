#ifndef _AC_ACSETTINGS_H
#define _AC_ACSETTINGS_H

// ac/acsettings.h
// 7/30/2011

#include <QMutex>
#include <QSettings>
#include <QStringList>
#include <QDate>
#include <QHash>
#include <utility>

class AcSettings : public QSettings
{
  Q_OBJECT
  typedef AcSettings Self;
  typedef QSettings Base;

  QMutex m_;

  // - Construction -
public:
  static Self *globalSettings() { static Self g; return &g; }
  explicit AcSettings(QObject *parent = 0);

  QString version() const;
  void setVersion(const QString &version);

public slots:
  void sync(); ///< thread-safe

public:
  // - Accounts -

  qint64 userId() const;
  void setUserId(qint64 uid);

  QString userName() const;
  void setUserName(const QString &userName);

  QString password() const; ///< Encrypted
  void setPassword(const QString &password);

  std::pair<QString, QString> nicovideoAccount();
  void setNicovideoAccount(const QString &username, const QString &password);

  std::pair<QString, QString> bilibiliAccount();
  void setBilibiliAccount(const QString &username, const QString &password);

  // - i18n -

  int language() const; ///< Application language
  void setLanguage(int lang);

  // - GUI -

  bool isAeroEnabled() const;
  void setAeroEnabled(bool t);

  bool isMenuThemeEnabled() const;
  void setMenuThemeEnabled(bool t);

  int themeId() const;
  void setThemeId(int tid);

  // - Network proxy -

  void setProxyEnabled(bool t);
  bool isProxyEnabled() const;

  void setProxyHostName(const QString &host);
  QString proxyHostName() const;

  void setProxyPort(int port);
  int proxyPort() const;

  void setProxyType(int type);
  int proxyType() const;

  void setProxyUser(const QString &name);
  QString proxyUser() const;

  void setProxyPassword(const QString &password);
  QString proxyPassword() const;
};

#endif // _AC_ACSETTINGS_H