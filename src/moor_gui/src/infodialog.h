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
#ifndef __INFODIALOG_H__
#define __INFODIALOG_H__
#include "qmoorie.h"
class QMoorie;
class InfoDialog : public QDialog
{
    Q_OBJECT
public :
    InfoDialog(QMoorie *parent);
    QLabel *info[11];
    QTextEdit *Hashcode;
    void getHashInfo();
    void close();
    QString hashCode, FileSize ;
};
#endif


