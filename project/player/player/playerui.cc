// playerui.cc
// 7/17/2011

#include "playerui.h"
#include "defines.h"
#include "stylesheet.h"
#include "tr.h"
#include "signalhub.h"
#include "module/player/player.h"
#include "module/serveragent/serveragent.h"
#include "core/util/datetime.h"
#include "core/cloud/annotation.h"
#ifdef Q_WS_WIN
  #include "win/qtwin/qtwin.h"
#endif // Q_WS_WIN
#include "boost/foreach.hpp"
#include <map>
#include <QtGui>

using namespace Core::Cloud;

/* jichi 7/25/2011: using boost is so bloated while less efficient than directly expanding the code.
#include <boost/typeof/typeof.hpp>
#include <boost/assign/list_of.hpp>
namespace { // anonymous
  // Player connections
  const QMap<const char*, const char*> &playerConnections_()
  {
    static QMap<const char*, const char*> connections
      (std::map<const char*, const char*>(boost::assign::map_list_of
        (SIGNAL(mediaChanged()), SLOT(invalidateTitle()))
        (SIGNAL(mediaChanged()), SLOT(invalidatePositionSlider()))
        (SIGNAL(mediaChanged()), SLOT(invalidatePositionButton()))
        (SIGNAL(encodingChanged()), SLOT(invalidateTitle()))
        (SIGNAL(lengthChanged()), SLOT(invalidatePositionButton()))
        (SIGNAL(timeChanged()), SLOT(invalidatePositionButton()))
        (SIGNAL(positionChanged()), SLOT(invalidatePositionSlider()))
        (SIGNAL(volumeChanged()), SLOT(invalidateVolumeSlider()))
        (SIGNAL(playing()), SLOT(invalidatePlayButton()))
        (SIGNAL(paused()), SLOT(invalidatePlayButton()))
        (SIGNAL(stopped()), SLOT(invalidatePlayButton()))
        (SIGNAL(playing()), SLOT(invalidateStopButton()))
        (SIGNAL(paused()), SLOT(invalidateStopButton()))
        (SIGNAL(stopped()), SLOT(invalidateStopButton()))
        (SIGNAL(playing()), SLOT(invalidateNextFrameButton()))
        (SIGNAL(paused()), SLOT(invalidateNextFrameButton()))
        (SIGNAL(stopped()), SLOT(invalidateNextFrameButton()))
      ));

    return connections;
  }

} // anonymous namespace

void
PlayerUi::connectPlayer()
{
  for (BOOST_AUTO(i, ::playerConnections_().begin()); i != ::playerConnections_().end(); ++i)
    connect(player_, i.key(), i.value());
}

void
PlayerUi::disconnectPlayer()
{
  for (BOOST_AUTO(i, ::playerConnections_().begin()); i != ::playerConnections_().end(); ++i)
    disconnect(player_, i.key(), i.value());
}
*/

 // - Constructions -

PlayerUi::PlayerUi(SignalHub *hub, Player *player, ServerAgent *server, QWidget *parent)
  : Base(parent), hub_(hub), player_(player), server_(server), active_(false)
{
  Q_ASSERT(isValid());

  // Post fix: remove blurred graphics effects for text buttons
  userButton()->setGraphicsEffect(0);

  toggleEmbedModeButton()->setCheckable(true);
  toggleMiniModeButton()->setCheckable(true);
  toggleFullScreenModeButton()->setCheckable(true);

  setTabOrder(inputComboBox(), prefixComboBox());
  setTabOrder(prefixComboBox(), inputComboBox());

  createConnections();

  inputComboBox()->setFocus();
}

void
PlayerUi::createConnections()
{
  connect(playButton(), SIGNAL(clicked()), SLOT(play()));
  connect(stopButton(), SIGNAL(clicked()), SLOT(stop()));
  connect(nextFrameButton(), SIGNAL(clicked()), SLOT(nextFrame()));

  connect(userButton(), SIGNAL(clicked()), SLOT(clickUserButton()));
  connect(positionButton(), SIGNAL(clicked()), SIGNAL(showPositionPanelRequested()));

  connect(positionSlider(), SIGNAL(valueChanged(int)), SLOT(setPosition(int)));
  connect(volumeSlider(), SIGNAL(valueChanged(int)), SLOT(setVolume(int)));

  connect(inputComboBox()->lineEdit(), SIGNAL(returnPressed()), SLOT(postAnnotation()));

  // Always connected
  connect(hub_, SIGNAL(tokenModeChanged(SignalHub::TokenMode)), SLOT(invalidateVisibleWidgets())); \

  //CommandLineEdit *p = dynamic_cast<CommandLineEdit*>(prefixLineEdit());
  //if (p)
  //  connect(lineEdit(), SIGNAL(returnPressed()), p, SLOT(addCurrentTextToHistory()));
}

#define CREATE_HUB_CONNECTIONS(_connect) \
  void \
  PlayerUi::_connect##Hub() \
  { \
    _connect(openButton(), SIGNAL(clicked()), hub_, SLOT(open())); \
    _connect(toggleFullScreenModeButton(), SIGNAL(clicked(bool)), hub_, SLOT(setFullScreenWindowMode(bool))); \
    _connect(toggleMiniModeButton(), SIGNAL(clicked(bool)), hub_, SLOT(setMiniPlayerMode(bool))); \
    _connect(toggleEmbedModeButton(), SIGNAL(clicked(bool)), hub_, SLOT(setEmbeddedPlayerMode(bool))); \
    _connect(hub_, SIGNAL(playModeChanged(SignalHub::PlayMode)), this, SLOT(invalidatePlayButton())); \
    _connect(hub_, SIGNAL(playModeChanged(SignalHub::PlayMode)), this, SLOT(invalidateStopButton())); \
    _connect(hub_, SIGNAL(playModeChanged(SignalHub::PlayMode)), this, SLOT(invalidatePositionSlider())); \
    _connect(hub_, SIGNAL(volumeChanged(qreal)), this, SLOT(invalidateVolumeSlider())); \
\
    static const char *status_signals[] = { \
      SIGNAL(played()), SIGNAL(paused()), SIGNAL(stopped()) \
    }; \
    static const char *invalidate_slots[] = { \
      SLOT(invalidatePlayButton()), \
      SLOT(invalidateStopButton()), \
    }; \
    BOOST_FOREACH (const char *signal, status_signals) \
      BOOST_FOREACH (const char *slot, invalidate_slots) \
        _connect(hub_, signal, this, slot); \
  }

  CREATE_HUB_CONNECTIONS(connect)
  CREATE_HUB_CONNECTIONS(disconnect)
#undef CREATE_HUB_CONNECTIONS

#define CREATE_PLAYER_CONNECTIONS(_connect) \
  void \
  PlayerUi::_connect##Player() \
  { \
    _connect(player_, SIGNAL(encodingChanged()), this, SLOT(invalidateTitle())); \
\
    _connect(player_, SIGNAL(mediaChanged()), this, SLOT(invalidateTitle())); \
    _connect(player_, SIGNAL(mediaClosed()), this, SLOT(invalidateTitle())); \
    _connect(player_, SIGNAL(mediaChanged()), this, SLOT(invalidatePositionSlider())); \
    _connect(player_, SIGNAL(mediaClosed()), this, SLOT(invalidatePositionSlider())); \
    _connect(player_, SIGNAL(mediaChanged()), this, SLOT(invalidatePositionButton())); \
    _connect(player_, SIGNAL(mediaClosed()), this, SLOT(invalidatePositionButton())); \
\
    _connect(player_, SIGNAL(lengthChanged()), this, SLOT(invalidatePositionButton())); \
    _connect(player_, SIGNAL(timeChanged()), this, SLOT(invalidatePositionButton())); \
    _connect(player_, SIGNAL(positionChanged()), this, SLOT(invalidatePositionSlider())); \
\
    _connect(player_, SIGNAL(playing()), this, SLOT(invalidateVolumeSlider())); \
 \
    static const char *status_signals[] = { \
      SIGNAL(playing()), SIGNAL(paused()), SIGNAL(stopped()) \
    }; \
    static const char *invalidate_slots[] = { \
      SLOT(invalidatePlayButton()), \
      SLOT(invalidateStopButton()), \
      SLOT(invalidateNextFrameButton()) \
    }; \
    BOOST_FOREACH (const char *signal, status_signals) \
      BOOST_FOREACH (const char *slot, invalidate_slots) \
        _connect(player_, signal, this, slot); \
  }

  CREATE_PLAYER_CONNECTIONS(connect)
  CREATE_PLAYER_CONNECTIONS(disconnect)
#undef CREATE_PLAYER_CONNECTIONS

#define CREATE_SERVER_CONNECTIONS(_connect) \
  void \
  PlayerUi::_connect##Server() \
  { \
    _connect(server_, SIGNAL(userChanged()), this, SLOT(invalidateUserButton())); \
  }

  CREATE_SERVER_CONNECTIONS(connect)
  CREATE_SERVER_CONNECTIONS(disconnect)
#undef CREATE_SERVER_CONNECTIONS

bool
PlayerUi::isValid() const
{ return hub_ && player_ && server_; }

void
PlayerUi::setTitle(const QString &title)
{ Base::setWindowTitle(title); }

void
PlayerUi::setVisible(bool visible)
{
  setActive(visible);
  Base::setVisible(visible);
}

void
PlayerUi::setActive(bool active)
{
  if (active) {
    invalidateTitle();
    invalidateVolumeSlider();
    invalidatePositionSlider();
    invalidatePositionButton();
    invalidatePlayButton();
    invalidateStopButton();
    invalidateNextFrameButton();
    invalidatePlayerModeToggler();
    invalidateWindowModeToggler();

    invalidateUserButton();

    if (!active_) {
      connectHub();
      connectPlayer();
      connectServer();
      active_ = true;
    }
  } else if (active_) {
    disconnectHub();
    disconnectPlayer();
    disconnectServer();
    active_ = false;
  }
}

// - Play control -

void
PlayerUi::play()
{
  switch (hub_->tokenMode()) {
  case SignalHub::LiveTokenMode:
  case SignalHub::SignalTokenMode:
    if (hub_->isPlaying())
      hub_->pause();
    else
      hub_->play();
    break;

  case SignalHub::MediaTokenMode:
    if (!player_->hasMedia())
      hub_->open();
    else if (player_->isValid())
      switch (player_->status()) {
      case Player::Playing:
        player_->pause();
        break;
      case Player::Paused:
        player_->resume();
        break;
      case Player::Stopped:
        player_->play();
        break;
      }
    break;
  }
}

void
PlayerUi::stop()
{
  switch (hub_->tokenMode()) {
  case SignalHub::SignalTokenMode:
    hub_->stop();
    break;

  case SignalHub::LiveTokenMode:
    hub_->stop();
    hub_->setMediaTokenMode();
    break;

  case SignalHub::MediaTokenMode:
    if (player_->hasMedia())
      switch (player_->status()) {
      case Player::Playing:
      case Player::Paused:
        player_->stop();
        break;

      case Player::Stopped:
        break;
      }
    break;
  }
}

void
PlayerUi::nextFrame()
{
  if (hub_->isMediaTokenMode() &&
      player_->hasMedia())
    player_->nextFrame();
}

// - Set/get properties -

SignalHub*
PlayerUi::hub() const
{ Q_ASSERT(hub_); return hub_; }

Player*
PlayerUi::player() const
{ Q_ASSERT(player_); return player_; }

ServerAgent*
PlayerUi::server() const
{ Q_ASSERT(server_); return server_; }

void
PlayerUi::setVolume(int vol)
{ hub_->setVolume(vol / G_VOLUME_MAX); }

void
PlayerUi::setPosition(int pos)
{
  if (player_->hasMedia())
    player_->setPosition(pos / G_POSITION_MAX);
}

void
PlayerUi::invalidateVolumeSlider()
{
  QSlider *slider = volumeSlider();

  if (!hub_->isMediaTokenMode() || !player_->isValid()) {
    slider->setSliderPosition(0);
    if (slider->isEnabled())
      slider->setEnabled(false);
//#ifdef Q_WS_WIN
//    QtWin::repaintWindow(slider->winId());
//#endif // Q_WS_WIN
    return;
  }

  if (!slider->isEnabled())
    slider->setEnabled(true);

  qreal vol = hub_->volume();
  slider->setSliderPosition(
    static_cast<int>(vol * G_VOLUME_MAX)
  );

  // Update tool tip.
  int percentage = qRound(vol * 100);
  slider->setToolTip(
    QString("%1 %2%")
      .arg(TR(T_VOLUME))
      .arg(QString::number(percentage))
  );

//#ifdef Q_WS_WIN
//  QtWin::repaintWindow(slider->winId());
//#endif // Q_WS_WIN
}

void
PlayerUi::invalidatePositionSlider()
{
  QSlider *slider = positionSlider();

  if (!hub_->isMediaTokenMode() || !player_->isValid()) {
    //slider->hide();
    slider->setSliderPosition(0);
    if (slider->isEnabled())
      slider->setEnabled(false);
//#ifdef Q_WS_WIN
//    QtWin::repaintWindow(slider->winId());
//#endif // Q_WS_WIN
    return;
  }

  //slider->show();

  if (player_->hasMedia()) {
    if (!slider->isEnabled())
      slider->setEnabled(true);

    if (!slider->isSliderDown()) {
      qreal pos = player_->position();
      slider->setSliderPosition(
        static_cast<int>(pos * G_POSITION_MAX)
      );
    }

    // Update slider's tool tip and label's text.
    qint64 current_msecs = player_->time(),
           total_msecs = player_->mediaLength();
    qint64 left_msecs = total_msecs - current_msecs;

    QTime current = Core::msecs2time(current_msecs),
          left = Core::msecs2time(left_msecs);

    slider->setToolTip(current.toString() + " / -" + left.toString());

  } else {
    if (slider->isEnabled())
      slider->setEnabled(false);

    QString zero = QTime(0, 0, 0).toString();
    slider->setToolTip(zero + " / -" + zero);
  }
#ifdef Q_WS_WIN
  //QtWin::repaintWindow(slider->winId());
#endif // Q_WS_WIN
}

void
PlayerUi::invalidateUserButton()
{
  QToolButton *button = userButton();
  if (server_->isAuthorized()) {
    button->setStyleSheet(SS_TOOLBUTTON_TEXT_NORMAL);
    button->setText(server_->user().name());
  } else {
    button->setStyleSheet(SS_TOOLBUTTON_TEXT_HIGHLIGHT);
    button->setText(TR(T_LOGIN));
  }
#ifdef Q_WS_WIN
  QtWin::repaintWindow(button->winId());
#endif // Q_WS_WIN
}

void
PlayerUi::invalidatePositionButton()
{
  QToolButton *button = positionButton();

  if (player_->hasMedia()) {
    if (!button->isEnabled())
      button->setEnabled(true);

    // Update slider's tool tip and button's text.
    qint64 current_msecs = player_->time(),
           total_msecs = player_->mediaLength();
    //qint64 left_msecs = total_msecs - current_msecs;
    //QTime left = Core::msecs2time(left_msecs),

    QTime current = Core::msecs2time(current_msecs),
          total = Core::msecs2time(total_msecs);

    button->setText(
      QString("%1 / %2")
        .arg(current.toString())
        .arg(total.toString())
    );

  } else {
    if (button->isEnabled())
      button->setEnabled(false);

    QString zero = QTime(0, 0, 0).toString();
    button->setText(zero + " / " + zero);
  }
#ifdef Q_WS_WIN
  QtWin::repaintWindow(button->winId());
#endif // Q_WS_WIN
}

void
PlayerUi::invalidateTitle()
{
  if (!hub_->isMediaTokenMode())
    return;
  if (player_->hasMedia()) {
    QString title = player_->mediaTitle();
    if (title.isEmpty()) {
      title = player_->mediaPath();
      if (!title.isEmpty())
        title = QFileInfo(title).fileName();
    }
    if (title.isEmpty())
      title = TR(T_TITLE_PROGRAM);

    setTitle(title);
  } else
    setTitle(TR(T_TITLE_PROGRAM));
}

void
PlayerUi::invalidatePlayButton()
{
  QToolButton *button = playButton();
  switch (hub_->tokenMode()) {
  case SignalHub::LiveTokenMode:
  case SignalHub::SignalTokenMode:
    if (hub_->isPlaying()) {
      button->setStyleSheet(SS_TOOLBUTTON_PAUSE);
      button->setToolTip(TR(T_TOOLTIP_PAUSE));
    } else {
      button->setStyleSheet(SS_TOOLBUTTON_PLAY);
      button->setToolTip(TR(T_TOOLTIP_PLAY));
    }
    break;

  case SignalHub::MediaTokenMode:
    switch (player_->status()) {
    case Player::Playing:
      button->setStyleSheet(SS_TOOLBUTTON_PAUSE);
      button->setToolTip(TR(T_TOOLTIP_PAUSE));
      break;
    case Player::Stopped:
    case Player::Paused:
      button->setStyleSheet(SS_TOOLBUTTON_PLAY);
      button->setToolTip(TR(T_TOOLTIP_PLAY));
      break;
    }
    break;
  }

#ifdef Q_WS_WIN
  QtWin::repaintWindow(button->winId());
#endif // Q_WS_WIN
}

void
PlayerUi::invalidateStopButton()
{
  QToolButton *button = stopButton();
  switch (hub_->tokenMode()) {
  case SignalHub::LiveTokenMode:
  case SignalHub::SignalTokenMode:
    button->setEnabled(!hub_->isStopped());
    break;

  case SignalHub::MediaTokenMode:
    button->setEnabled(!player_->isStopped());
    break;
  }
#ifdef Q_WS_WIN
  QtWin::repaintWindow(button->winId());
#endif // Q_WS_WIN
}

void
PlayerUi::invalidateNextFrameButton()
{
  QToolButton *button = nextFrameButton();
  if (!hub_->isMediaTokenMode()) {
    button->hide();
    button->setEnabled(false);
    return;
  }

  button->show();

  switch (player_->status()) {
  case Player::Playing:
  case Player::Paused:
    button->setEnabled(true);
    break;
  case Player::Stopped:
    button->setEnabled(false);
    break;
  }
#ifdef Q_WS_WIN
  QtWin::repaintWindow(button->winId());
#endif // Q_WS_WIN
}

// - Annotations -

void
PlayerUi::postAnnotation()
{
  QString text = inputComboBox()->currentText();
  if (text.isEmpty())
    return;

  inputComboBox()->clearEditText();

  QString prefix = prefixComboBox()->currentText();
  if (!prefix.trimmed().isEmpty()) {
    text = prefix + " " + text;

    QKeyEvent e(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QCoreApplication::sendEvent(prefixComboBox()->lineEdit(), &e);
  }

  // TODO: use command filter rather than annnotation filter here
  if (Annotation::isValidText(text))
    emit textEntered(text);
}

// - Requests -

void
PlayerUi::clickUserButton()
{
  if (!server_->isAuthorized())
    emit loginRequested();
  else {
    emit invalidateUserMenuRequested();
    userButton()->showMenu();
  }
}

void
PlayerUi::setAnnotationEnabled(bool enabled)
{
  QToolButton *button = toggleAnnotationButton();
  if (enabled) {
    button->setStyleSheet(SS_TOOLBUTTON_HIDEANNOT);
    button->setToolTip(TR(T_TOOLTIP_HIDEANNOT));
  } else {
    button->setStyleSheet(SS_TOOLBUTTON_SHOWANNOT);
    button->setToolTip(TR(T_TOOLTIP_SHOWANNOT));
  }
#ifdef Q_WS_WIN
  QtWin::repaintWindow(button->winId());
#endif // Q_WS_WIN
}

void
PlayerUi::invalidatePlayerModeToggler()
{
  toggleMiniModeButton()->setChecked(hub_->isMiniPlayerMode());
  toggleEmbedModeButton()->setChecked(hub_->isEmbeddedPlayerMode());
}

void
PlayerUi::invalidateWindowModeToggler()
{ toggleFullScreenModeButton()->setChecked(hub_->isFullScreenWindowMode()); }

void
PlayerUi::invalidateVisibleWidgets()
{
  bool v = hub_->isMediaTokenMode();
  volumeSlider()->setVisible(v);
  positionSlider()->setVisible(v);
  positionButton()->setVisible(v);

  previousButton()->setVisible(v);
  nextButton()->setVisible(v);
  nextFrameButton()->setVisible(v);
}

// EOF
