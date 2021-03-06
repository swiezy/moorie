/*
 *   Copyright (C) 2008-2010 by Patryk Połomski
 *   cykuss@gmail.com
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef MYSYSTEMTRAYICON_H
#define MYSYSTEMTRAYICON_H

#if defined(unix)
#include <QDBusInterface>
#include <QDBusReply>
#endif
#include <QMouseEvent>
#include <QSystemTrayIcon>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include "singleton.h"

class diall : public QFrame
{
    Q_OBJECT
    void mousePressEvent(QMouseEvent * event);
public :
    diall(QRect pos, QWidget *parent = 0);
    QTimer *timer;
    QLabel *label;
    QRect pos;
    void setPosition();
public Q_SLOTS:
    void closeD();
};

class mySystemTrayIcon: public QSystemTrayIcon
{
#if defined(unix)
    QDBusInterface *KNotify;
#endif
    bool UseFreedesktopStandard;
public:
    mySystemTrayIcon(QWidget *parent = 0);
    void showHints(QString, QString, int seconds = 10);

};

#endif MYSYSTEMTRAYICON_H
