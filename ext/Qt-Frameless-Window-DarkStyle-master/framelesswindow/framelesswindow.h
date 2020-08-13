/*
###############################################################################
#                                                                             #
# The MIT License                                                             #
#                                                                             #
# Copyright (C) 2017 by Juergen Skrotzky (JorgenVikingGod@gmail.com)          #
#               >> https://github.com/Jorgen-VikingGod                        #
#                                                                             #
# Sources: https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle  #
#                                                                             #
###############################################################################
*/

#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#include <QEvent>
#include <QtWidgets>

namespace Ui {
  class FramelessWindow;
}

class MouseButtonSignaler: public QObject
{
  Q_OBJECT

public:
  MouseButtonSignaler(QObject * parent = 0) : QObject(parent) {}
  void installOn(QWidget * widget) { widget->installEventFilter(this); }

protected:
  virtual bool eventFilter(QObject * obj, QEvent * ev) Q_DECL_OVERRIDE {
    if ((   ev->type() == QEvent::MouseButtonPress
         || ev->type() == QEvent::MouseButtonRelease
         || ev->type() == QEvent::MouseButtonDblClick)
        && obj->isWidgetType()) {
      emit mouseButtonEvent(static_cast<QWidget*>(obj),
                            static_cast<QMouseEvent*>(ev));
    }
    return false;
  }
signals:
  void mouseButtonEvent(QWidget *, QMouseEvent *);
};

class FramelessWindow: public QWidget
{
  Q_OBJECT

public:
  explicit FramelessWindow(QWidget *parent = 0);
  void setContent(QWidget *w);

  // Set a content dialog which if the close button is done, it sends a cancel signal.
  void ContentDlg(QDialog* indlg);
  void SetTitleBarBtns(bool Maximize,bool Minimize,bool Close);
private:
  void styleWindow(bool bActive, bool bNoState);

  bool ContDlg;
  QDialog* dlgCont;

signals:
  void windowIconLeftClicked();
  void windowIconRightClicked();
  void windowIconDblClick();

public slots:
  void setWindowTitle(const QString &text);
  void setWindowIcon(const QIcon &ico);

private slots:
  void on_applicationStateChanged(Qt::ApplicationState state);
  void on_minimizeButton_clicked();
  void on_restoreButton_clicked();
  void on_maximizeButton_clicked();
  void on_closeButton_clicked();
  void on_windowTitlebar_doubleClicked();

protected:
  virtual void changeEvent(QEvent *event);

private:
  Ui::FramelessWindow *ui;

protected:
  QHBoxLayout contentLayout;
};

#endif // FRAMELESSWINDOW_H
