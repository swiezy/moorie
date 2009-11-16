/*
 *   Copyright (C) 2008-2009 by Patryk Połomski
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
#include "qmoorie.h"


QMoorie::QMoorie(QWidget * parent, Qt::WFlags f):QMainWindow(parent, f), ui(new Ui::MainWindow)
{
    setWindowIcon( QIcon(":/images/hi16-app-qmoorie.png"));
    setWindowTitle(qApp->applicationName()  + " " + qApp->applicationVersion() + " - Hashcode Downloader");

    ui->setupUi(this);
    QTextCodec::setCodecForTr (QTextCodec::codecForName ("UTF-8"));
    QTextCodec::setCodecForCStrings ( QTextCodec::codecForName ("UTF-8"));
    QTextCodec::setCodecForLocale ( QTextCodec::codecForName ("UTF-8"));

    createTable();
    createActions();
    createToolBars();
    readConfigFile();

    setTray();
    setLog();
    statusesThread = new boost::thread( boost::bind( &QMoorie::refreshStatuses, this ) );
}

void QMoorie::setTray()
{
    tray = new QSystemTrayIcon();
    tray->setIcon(QIcon(":/images/hi64-app-qmoorie.png") );
    connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    this,SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
    contextMenu = new QMenu();
    contextMenu -> addAction(addAct);
    contextMenu -> addAction(settingsAct);
    contextMenu -> addSeparator();
    contextMenu -> addAction(exitAct);
    tray -> setContextMenu(contextMenu);
    if(Zmienne().TRAY) tray->show();

}
void QMoorie::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) toggleVisibility();
}
void QMoorie::toggleVisibility()
{
    if(isHidden())
    {
        show();
        if(isMinimized())
        {
            if(isMaximized()) showMaximized();
            else showNormal();
        }
        raise();
        activateWindow();
    }
    else hide();
}
 void QMoorie::createActions()
 {

     addAct = new QAction(QIcon(":/images/add.png"), tr("&Dodaj"), this);
     addAct->setShortcut(tr("Ctrl+N"));
     addAct->setStatusTip(tr("Rozpoczyna pobieranie nowego pliku"));
     connect(addAct, SIGNAL(triggered()), this, SLOT(addDialog()));

     settingsAct = new QAction(QIcon(":images/tool2.png"),tr("&Ustawienia"),this);
     settingsAct -> setShortcut(tr("Ctrl+Alt+S"));
     settingsAct -> setStatusTip(tr("Konfiguracja aplikacji"));
     connect(settingsAct, SIGNAL(triggered()), this ,SLOT(showSettings()));

     playAct = new QAction(QIcon(":images/play.png"),tr("&Wznów"),this);
     playAct -> setStatusTip(tr("Wznawia pobieranie"));

     pauseAct = new QAction(QIcon(":images/pause.png"),tr("Wstrzymaj"),this);
     pauseAct -> setStatusTip(tr("Wstrzymuje pobieranie"));

     removeAct = new QAction(QIcon(":images/remove.png"),tr("&Usuń"),this);
     removeAct -> setStatusTip(tr("Usuwa pobierany plik"));

     aboutAct = new QAction(QIcon(":/images/help_about.png"), tr("&O programie"), this);
     aboutAct->setStatusTip(tr("Pokaż informacje o aplikacji"));
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));

     exitAct = new QAction(QIcon(":/images/exit.png"), tr("Wyjście"), this);
     exitAct->setShortcut(tr("Ctrl+Q"));
     exitAct->setStatusTip(tr("Wyjście z aplikacji"));
     connect(exitAct, SIGNAL(triggered()), this, SLOT(exitApp()));

     connect(tabela->tInfoAct, SIGNAL(triggered()), this, SLOT(infoDialog()));
     connect(&tLogs, SIGNAL(append(const QString &)), ui->log, SLOT(append(const QString&)));
}

 void QMoorie::createToolBars()
 {
     fileToolBar = addToolBar(tr("Plik"));
     fileToolBar->addAction(addAct);
     fileToolBar->addAction(removeAct);
     fileToolBar->addSeparator();
     fileToolBar->addAction(playAct);
     fileToolBar->addAction(pauseAct);
     fileToolBar->addAction(settingsAct);
     fileToolBar->addSeparator();
     fileToolBar->addAction(exitAct);
     fileToolBar->addAction(aboutAct);
     fileToolBar->addSeparator();
     fileToolBar->addAction(exitAct);
     fileToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
     fileToolBar->setIconSize(QSize(24,24));
 }
void QMoorie::createTable()
{
    tabela = new myTableWidget();
    QVBoxLayout *main = new QVBoxLayout;
    main -> addWidget(tabela);
    ui->tab_2->setLayout(main);
    tabela->setEditTriggers(0);
    tabela->setItemDelegate(new TrackDelegate());
    tabela->setColumnCount(7);
    headarH  << "Nazwa pliku" << "Rozmiar" << "Pozostało"<< "Stan Pobierania " << "Prędkość" << "Status" << "Skrzynka";
    tabela->setHorizontalHeaderLabels(headarH);
}
void QMoorie::addDialog()
{
    addDownload *get = new addDownload(this);
    get->exec();
    if(get->result())addInstance(get->text->toPlainText(), get->pathEdit->text());
    delete get;
}

/**
* Tworzymy nową instację pobierania
* @param hash zawiera hashcode
* @param path zawiera ścieżkę pobierania
*/
void QMoorie::addInstance(QString hash, QString path)
{
    tInstance.append(new threadInstance(hash, path));
    tInstance.last()->start();
    const MoorhuntHash & hashcode (tInstance.last()->hashcode.toStdString());
    tInstance.last()->filename = QString::fromStdString(hashcode.getFileName());
    tInstance.last()->size = hashcode.getFileSize();
    tInstance.last()->pobrano = false;
    tInstance.last()->itemRow = tabela->rowCount();

    tabela->setRowCount(tabela->rowCount() + 1);

    QString fileSize; fileSize.sprintf("%.2f", tInstance.last()->size / 1024 / 1024);

    QTableWidgetItem *nazwaPliku = new QTableWidgetItem(tInstance.last()->filename);
    QTableWidgetItem *rozmiarPliku = new QTableWidgetItem(fileSize + " MB");
    tabela->setItem(tInstance.last()->itemRow, 0, nazwaPliku);
    tabela->setItem(tInstance.last()->itemRow, 1, rozmiarPliku);
}
void QMoorie::refreshStatuses()
{

    while(1)
    {
        sleep(2);
        for (int i = 0; i < tInstance.size(); ++i) {
                QString fileSize; fileSize.sprintf("%.2f", (tInstance.at(i)->size - tInstance.at(i)->vInstance.getBytesRead())  / 1024 / 1024);

                QTableWidgetItem *PobranoPliku = new QTableWidgetItem(fileSize + " MB");
                tabela->setItem(tInstance.at(i)->itemRow, 2, PobranoPliku);
                int percentDownloaded = 100.0f * tInstance.at(i)->vInstance.getBytesRead()  / tInstance.at(i)->size;
                QTableWidgetItem *stanPobieraniaPliku = new QTableWidgetItem;
                stanPobieraniaPliku->setData(Qt::DisplayRole, percentDownloaded);
                tabela->setItem(tInstance.at(i)->itemRow, 3, stanPobieraniaPliku);

                const double speed( static_cast<double>( tInstance.at(i)->vInstance.getSpeed()) / 1024.0f );
                std::stringstream s;
                s.setf( std::ios::fixed);
                if ( speed - round( speed ) < 0.1f )
                {
                    s << std::setprecision( 0 );
                }
                else
                {
                    s << std::setprecision( 2 );
                }
                s << speed;
                QTableWidgetItem *SzybkoscPobierania = new QTableWidgetItem(QString::fromStdString(s.str()) + " KB/s");
                tabela->setItem(tInstance.at(i)->itemRow, 4, SzybkoscPobierania);


        }
    }
}
void QMoorie::setLog()
{
    tLogs.setLogLevel(Zmienne().LLEVEL);
    tLogs.start();
}
void QMoorie::aboutDialog()
{
    about *get = new about(this);
    get->exec();
    delete get;
}
void QMoorie::infoDialog()
{
    InfoDialog *get = new InfoDialog(this);
    get->exec();
    delete get;
}
void QMoorie::showSettings()
{
    ConfigDialog *get = new ConfigDialog();
    get->exec();
    if(!Zmienne().TRAY) tray->hide();
    else tray->show();
    delete get;
}
void QMoorie::readConfigFile()
{
    QHeaderView *header = tabela->horizontalHeader();
    QSettings settings;
    settings.beginGroup("CONFIG_PAGE");
    Zmienne().PATH = settings.value("PATH", "home").toString();
    Zmienne().LLEVEL = settings.value("LLEVEL", 8).toInt();
    Zmienne().DLEVEL = settings.value("DLEVEL", 2).toInt();
    Zmienne().KSEGMENTS = settings.value("KSEGMENTS", 1).toBool();
    Zmienne().TRAY = settings.value("TRAY", true).toBool();
    settings.endGroup();

    settings.beginGroup("GEOMETRY_QMOORIE");
    resize(settings.value("size", QSize(848, 280)).toSize());
    move(settings.value("pos", QPoint(0, 0)).toPoint());
    header->resizeSection( 0, settings.value("tabela_column_0", 260).toInt() );
    header->resizeSection( 1, settings.value("tabela_column_1", 80).toInt() );
    header->resizeSection( 2, settings.value("tabela_column_2", 70).toInt() );
    header->resizeSection( 3, settings.value("tabela_column_3", 120).toInt() );
    header->resizeSection( 4, settings.value("tabela_column_4", 65).toInt() );
    header->resizeSection( 5, settings.value("tabela_column_5", 120).toInt() );
    header->resizeSection( 6, settings.value("tabela_column_6", 100).toInt() );
    settings.endGroup();

    writeConfigFile();
}
void QMoorie::writeConfigFile()
{
    QHeaderView *header = tabela->horizontalHeader();
    QSettings settings;

    if(settings.isWritable()){
        settings.beginGroup("CONFIG_PAGE");
        settings.setValue("PATH", Zmienne().PATH);
        settings.setValue("LLEVEL", Zmienne().LLEVEL);
        settings.setValue("DLEVEL", Zmienne().DLEVEL);
        settings.setValue("KSEGMENTS", Zmienne().KSEGMENTS);
        settings.setValue("TRAY", Zmienne().TRAY);
        settings.endGroup();

        settings.beginGroup("GEOMETRY_QMOORIE");
        settings.setValue("size", size());
        settings.setValue("pos", pos());
        settings.setValue("tabela_column_0", header->sectionSize(0));
        settings.setValue("tabela_column_1", header->sectionSize(1));
        settings.setValue("tabela_column_2", header->sectionSize(2));
        settings.setValue("tabela_column_3", header->sectionSize(3));
        settings.setValue("tabela_column_4", header->sectionSize(4));
        settings.setValue("tabela_column_5", header->sectionSize(5));
        settings.setValue("tabela_column_6", header->sectionSize(6));
        settings.endGroup();
    }
    else
    {
        QMessageBox::critical(NULL, "QMoorie", "Nie można zapisać pliku konfiguracyjnego do\n "+settings.fileName(), "OK");
        this->close();
    }
}
void QMoorie::closeEvent(QCloseEvent *event)
{
    writeConfigFile();
    if(Zmienne().TRAY){
        event->ignore();
        hide();
    }
    else qApp->quit();
}
void QMoorie::exitApp()
{
    writeConfigFile();
    qApp->quit();
}
QMoorie::~QMoorie()
{
}
