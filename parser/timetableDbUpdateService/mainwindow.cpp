#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QIODevice>
#include <QByteArray>
#include <QTextCodec>
#include <QTextStream>
#include <QTimer>
#include <QSqlError>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadDefinitions();
    setWindowIcon(QIcon(":/img/icon.png"));
    needShow = (definitions->at("optionHideToTray") != "true");
    this -> setTrayIconActions();
    dlg = new myDlg(this);
    connect(dlg->ui->repeat, SIGNAL(clicked(bool)), this, SLOT(onRepeat()));
    connect(dlg->ui->quit, SIGNAL(clicked(bool)), this, SLOT(close()));
    this -> showTrayIcon();
    db = QSqlDatabase::addDatabase(definitions->at("sqlType"));
    db.setHostName(definitions->at("sqlHostName"));
    db.setPort(definitions->at("sqlPort").toInt());
    db.setDatabaseName(definitions->at("sqlDatabaseName"));
    db.setUserName(definitions->at("sqlUserName"));
    db.setPassword(definitions->at("sqlPassword"));
    if(!db.open())
    {
        qDebug() << db.lastError().text();
        dlg->ui->label->setText(db.lastError().text());
        dlg->show();
        return;
    }
    else
        qDebug() << "connection to database. success!";
    clearBase();
    //инициализация кодека
    codec = QTextCodec::codecForName("Windows-1251");
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    timer = new QTimer(this);
    setWindowTitle("Парсер");
    setFixedSize(400, 150);
    QString s; s.sprintf("%d", 328);
    ui->progressBar->setMaximum(328);
    ui->total->setText(s);
    s.sprintf("%d", 0);
    ui->done->setText(s);
    tree = new Tree;
    //подключение сигнала о завершении загрузки
    connect(this, SIGNAL(loadDone()), this, SLOT(onLoadDone()));
    connect(this, SIGNAL(allDone()), this, SLOT(onAllDone()));
    myUrl = definitions->at("targetUrl");
    //myUrl = "http://localhost/select.php";
    //загрузка корня дерева вариантов
    doDownload(myUrl.toStdString().c_str(), "");
}
MainWindow::~MainWindow()
{
    delete timer;
    delete dlg;
    delete ui;
    delete tree;
    delete definitions;
}
int MainWindow::parse(int prnt)
{
    //новый вариант
    Tree::item item;
    //полечение строки из буфера
    QString str = codec->toUnicode(buf);
    //ключевая фраза (key),
    //имя параметра (name),
    //значение (value),
    //содержимое (contain)
    QString key, name, value, contain;
    //длинна подстроки (len),
    //индекс конца тега select (selEnd)
    int selEnd;
    //текущий индекс (index)
    //начало тега select
    int index = str.indexOf("<select");
    if(index == -1)
    {
        //qDebug() << "tag select not found";
        completeTheBranch(str, prnt);
        readTheTable(str, prnt);
        return prnt;
    }
    //получаем имя параметра
    name = getPhrse(str, "name=\"", "\"", index);

    item.name = name;
    //qDebug() << name;

    key = "</select>";
    //конец тега select
    selEnd = str.indexOf(key, index);
    //разбор вариантов
    //пока варианты есть и индекс не достиг конца тега select
    bool done = false;
    while(!done)
    {
        //получаем значение
        value = getPhrse(str, "<option value=\"", "\"", index);
        if(index == -1 || index >= selEnd){
            done = true;
            continue;
        }
        item.values->push_back(value);
        //получаем содержимое
        contain = getPhrse(str, ">", "<", index);
        item.contain->push_back(contain);
        //qDebug() << value << contain;

        key = "<option value=\"";
        index = str.indexOf(key, index);
        if(value.length() == 0)
        {
            //qDebug() << "value not found";
            completeTheBranch(str, prnt);
            return prnt;
        }
    }

    item.parent = prnt;
    if(prnt != -1) item.id = (*tree->items)[prnt].children++;
    //помещаем вариант в хранилище
    tree->items->push_back(item);
    int pointer = tree->items->size() - 1;
    qDebug() << "parse done";
    return pointer;
}

QString MainWindow::getPhrse(const QString &source, const QString &beginsOn, const QString &endsOn, int &index)
{
    index = source.indexOf(beginsOn, index);
    if(index == -1) return "";
    index += beginsOn.length();
    int len = source.indexOf(endsOn, index) - index;
    index += len;
    return source.mid(index - len, len);
}

void MainWindow::completeTheBranch(const QString &str, int prnt)
{
    //qDebug() << "completeTheBranch called, parent index =" << prnt;
    bool done = false;
    int index = str.indexOf("<td colspan=\"5\">");
    int stopInd = str.indexOf("<p><a", index);
    std::vector<QString> names, values;
    QString name, value;
    while(!done)
    {
        name = getPhrse(str, "<p>", ":", index);
        if(index == -1 || index >= stopInd)
        {
            done = true;
            continue;
        }
        names.push_back(name);
        value = getPhrse(str, "<b>", "</b>", index);
        values.push_back(value);
    }
    int a = prnt, i = values.size() - 1;
    while(a != -1 && i >= 0)
    {
        Tree::item &it = (*tree->items)[a];
        //if(it.tId != "") break;
        it.table = names[i];
        it.tVal = values[i];
        it.tId = putToBase(it.table, it.tVal);
        i--;
        a = (*tree->items)[a].parent;
    }
}

void MainWindow::readTheTable(const QString &str, int prnt)
{
    //qDebug() << "readTheTable called";

    int a = prnt;
    QString interval_id = (*tree->items)[a].tId;
    a = (*tree->items)[a].parent;
    QString group_id = (*tree->items)[a].tId;
    a = (*tree->items)[a].parent;
    QString kurs_id = (*tree->items)[a].tId;
    a = (*tree->items)[a].parent;
    QString fak_id = (*tree->items)[a].tId;
    a = (*tree->items)[a].parent;
    QString form_id = (*tree->items)[a].tId;

    int index = str.indexOf("<b>Группа");
    QString firstWeek = getPhrse(str, "(с ", " п", index);
    QString secondWeek = getPhrse(str, "о ", " н", index);

    QString date, time, obj, room, teacher;
    QString day_id, time_id, obj_id, room_id, teacher_id;
    QString day;
    int tmp;
    index = str.indexOf("<td colspan=5>", index);
    for(int cDay = 1; cDay <= 12; cDay++)
    {
        if(index == -1) break;
        day.sprintf("%d", (cDay < 7 ? cDay : cDay - 6));

        date = getPhrse(str, " (", ")", index);
        day_id = putDayToBase(day, dateFormat(date), (cDay < 7 ? firstWeek : secondWeek));
        tmp = str.indexOf("<td colspan=5>", index);
        bool done = false;
        while(!done)
        {
            time = getPhrse(str, "<td>", "</td>", index);
            if(index == -1 || tmp != -1 && index >= tmp)
            {
                done = true;
                continue;
            }
            time_id = putToBase(definitions->at("tableTime"), time);
            obj = getPhrse(str, "<td>", "</td>", index);
            obj_id = putToBase(definitions->at("tableObj"), obj);

            getPhrse(str, "<td>", "</td>", index);

            room = getPhrse(str, "<td>", "</td>", index);
            room_id = putToBase(definitions->at("tableRoom"), room);
            teacher = getPhrse(str, "<td>", "</td>", index);
            teacher_id = putToBase(definitions->at("tableTeacher"), teacher);

            putPairToBase(form_id, fak_id, kurs_id, group_id, interval_id,
                          day_id, time_id, obj_id, room_id, teacher_id);
        }
        index = tmp;
    }
}

QString MainWindow::putToBase(const QString &table, const QString &value)
{
    QString disp = QString("putToBase table ") + table + " value " + value;
    ui->label_3->setText(disp);
    qDebug() << "putToBase table" << table << "value" << value << endl;
    QSqlQuery query;
    QString str;
    str = definitions->at("queryCheckTable");
    str.sprintf(str.toUtf8().data(),
                table.toUtf8().data());
    query.exec(str);
    if(query.size() == 0)
    {
        query.clear();
        str = definitions->at("queryCreateStandartTable");
        str.sprintf(str.toUtf8().data(),
                    table.toUtf8().data());
        query.exec(str);
        query.clear();
    }
    str = definitions->at("querySelectIdFromStandartTable");
    str.sprintf(str.toUtf8().data(),
                table.toUtf8().data(),
                value.toUtf8().data());
    query.exec(str);
    //qDebug() << "query.size() =" << query.size();
    if(query.size() == 0)
    {
        query.clear();
        str = definitions->at("queryInsertToStandartTable");
        str.sprintf(str.toUtf8().data(),
                    table.toUtf8().data(),
                    value.toUtf8().data());
        query.exec(str);
        query.clear();
        str = definitions->at("querySelectIdFromStandartTable");
        str.sprintf(str.toUtf8().data(),
                    table.toUtf8().data(),
                    value.toUtf8().data());
        query.exec(str);
    }
    query.seek(0);
    return query.value(0).toString();
}

QString MainWindow::putDayToBase(const QString &day, const QString &date, const QString &week)
{
    QSqlQuery query;
    QString str;
    str = definitions->at("queryCheckTable");
    str.sprintf(str.toUtf8().data(),
                definitions->at("tableDay").toUtf8().data());
    query.exec(str);
    if(query.size() == 0)
    {
        query.clear();
        str = definitions->at("queryCreateDayTable");
        query.exec(str);
        query.clear();
    }
    str = definitions->at("querySelectIdFromDayTable");
    str.sprintf(str.toUtf8().data(),
                day.toUtf8().data(),
                date.toUtf8().data(),
                week.toUtf8().data());
    query.exec(str);
    if(query.size() == 0)
    {
        query.clear();
        str = definitions->at("queryInsertToDayTable");
        str.sprintf(str.toUtf8().data(),
                    day.toUtf8().data(),
                    date.toUtf8().data(),
                    week.toUtf8().data());
        query.exec(str);
        query.clear();
        str = definitions->at("querySelectIdFromDayTable");
        str.sprintf(str.toUtf8().data(),
                    day.toUtf8().data(),
                    date.toUtf8().data(),
                    week.toUtf8().data());
        query.exec(str);
    }
    query.seek(0);
    return query.value(0).toString();
}

void MainWindow::putPairToBase(const QString &form, const QString &fak,
                               const QString &kurs, const QString &group,
                               const QString &interval, const QString &day,
                               const QString &time, const QString &obj,
                               const QString &room, const QString &teacher)
{
    QSqlQuery query;
    QString str;
    str = definitions->at("queryCheckTable");
    str.sprintf(str.toUtf8().data(),
                definitions->at("tablePair").toUtf8().data());
    query.exec(str);
    if(query.size() == 0)
    {
        query.clear();
        str = definitions->at("queryCreatePairTable");
        query.exec(str);
        query.clear();
    }
    str = definitions->at("queryInsertToPairTable");
    str.sprintf(str.toUtf8().data(),
                form.toUtf8().data(), fak.toUtf8().data(),
                kurs.toUtf8().data(), group.toUtf8().data(),
                interval.toUtf8().data(), day.toUtf8().data(),
                time.toUtf8().data(), obj.toUtf8().data(),
                room.toUtf8().data(), teacher.toUtf8().data());
    query.exec(str);
}

QString MainWindow::dateFormat(const QString &date)
{
    QString day, mon, year;
    day = date.mid(0, 2);
    mon = date.mid(3, 2);
    year = date.mid(6, 4);
    return year + "-" + mon + "-" + day;
}

void MainWindow::loadDefinitions()
{
    definitions = new Definitions;
    QFile file("C:/definitions.ini");
    //if(!file.open(QIODevice::ReadOnly))
    //    file.setFileName("C:/Projects/timetableDbUpdateService/definitions.ini");
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Не удалось открыть файл definitions.ini";
        exit(2);
    }
    QByteArray arr = file.readAll();
    QString str = arr.data();
    file.close();
    bool done = false;
    int index = 1;
    QString key, value;
    while(!done)
    {
        key = getPhrse(str, "->", "=", --index);
        if(index == -1)
        {
            done = true;
            continue;
        }
        value = getPhrse(str, "=", "<-", --index);
        DsPair p(key, value);
        definitions->insert(p);
    }
    qDebug() << definitions->size() << "definitions loaded";
}

void MainWindow::clearBase()
{
    QSqlQuery query1, query2;
    QString str;
    str = definitions->at("queryShowTables");
    query1.exec(str);
    while (query1.next())
    {
        str = definitions->at("queryClearTable");
        str.sprintf(str.toUtf8().data(),
                    query1.value(0).toString().toUtf8().data());
        query2.exec(str);
        query2.clear();
    }
}
void MainWindow::onLoadDone()
{
    static bool needPause = false;
    needPause = !needPause;
    if(needPause)
    {
        timer->setInterval(500);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(onLoadDone()));
        timer->start();
        return;
    }
    else
    {
        disconnect(timer, SIGNAL(timeout()), this, SLOT(onLoadDone()));
    }
    static int current = -1;
    int old = current;
    current = parse(current);
    //разбор свежих данных
    int cdone, cvaluessize;
    if(old == current)
    {
        (*tree->items)[current].done++;
        cdone = (*tree->items)[current].done;
        cvaluessize = (*tree->items)[current].values->size();
        while(cdone == cvaluessize)
        {
            if((*tree->items)[current].parent == -1)
            {
                emit allDone();
                return;
            }
            current = (*tree->items)[current].parent;
            (*tree->items)[current].done++;
            cdone = (*tree->items)[current].done;
            cvaluessize = (*tree->items)[current].values->size();
        }
    }
    //qDebug() << "current->name =" << (*tree->items)[current].name;
    QString param = "";
    int a;
    a = current;
    while(a != -1){
        if(param.length() != 0) param += "&";
        param += (*tree->items)[a].name + "=" +
                (*(*tree->items)[a].values)[(*tree->items)[a].done];
        a = (*tree->items)[a].parent;
    }
    //qDebug() << param;
    int tmpsize = tree->items->size();
    QString s = ""; s.sprintf("%d", tmpsize);
    if(ui->progressBar->maximum() < tmpsize)
    {
        ui->progressBar->setMaximum(tmpsize);
        ui->total->setText(s);
    }
    ui->progressBar->setValue(tmpsize);
    ui->done->setText(s);
    doDownload(myUrl.toStdString().c_str(), codec->fromUnicode(param).data());
}
void MainWindow::onAllDone()
{
    qDebug() << "AllDone!";
    qDebug() << "tree size =" << tree->items->size();
    if(isHidden()) close();
    else ui->label_5->setText("Загрузка завершена");
}
void MainWindow::onServerInaccessible()
{
    dlg->show();
    qDebug() << "Server Inaccessible";
}
void MainWindow::onRepeat()
{
    dlg->hide();
    dlg->ui->label->setText("Сервер не доступен");
    doDownload(codec->fromUnicode(address),
               codec->fromUnicode(params));
}
void MainWindow::onTimeout()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    dlg->ui->label->setText("Время ожидания превышено");
    dlg->show();
    currentReply->abort();
    qDebug() << "Timeout";
}
void MainWindow::doDownload(const char *url, const char *param)
{
    address = url;
    params = param;
    timer->setInterval(30000);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start();
    currentReply = manager->post(QNetworkRequest(QUrl(url)), QByteArray(param));
}
void MainWindow::replyFinished (QNetworkReply *reply)
{
    timer->stop();
    disconnect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
        onServerInaccessible();
    }
    else
    {
        buf.clear();
        buf = reply->readAll();
        reply->deleteLater();
        emit loadDone();
    }
}
void MainWindow::on_abort_clicked()
{
    close();
}

void MainWindow::on_toBackgraund_clicked()
{
    hide();
}

void MainWindow::on_info_clicked()
{
    dlg->ui->repeat->hide();
    disconnect(dlg->ui->quit, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(dlg->ui->quit, SIGNAL(clicked(bool)), this, SLOT(onInfoDlgQuit()));
    dlg->setModal(false);
    dlg->ui->label->setText("INFO PLACEHOLDER");
    dlg->show();
}

void MainWindow::onInfoDlgQuit()
{
    dlg->hide();
    dlg->ui->repeat->show();
    disconnect(dlg->ui->quit, SIGNAL(clicked(bool)), this, SLOT(onInfoDlgQuit()));
    connect(dlg->ui->quit, SIGNAL(clicked(bool)), this, SLOT(close()));
    dlg->setModal(true);
    dlg->ui->label->setText("Сервер не доступен");
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event -> type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            this -> hide();
        }
    }
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        this -> trayActionExecute();
        break;
    default:
        break;
    }
}

void MainWindow::trayActionExecute()
{
    if(isMinimized())
    {
        setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    }
    else if(!isVisible())
    {
        show();
        activateWindow();
    }
    else
    {
        hide();
    }
}

void MainWindow::setTrayIconActions()
{
    // Setting actions...
    minimizeAction = new QAction("Свернуть", this);
    restoreAction = new QAction("Восстановить", this);
    quitAction = new QAction("Выход", this);

    // Connecting actions to slots...
    connect (minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    connect (restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    connect (quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    // Setting system tray's icon menu...
    trayIconMenu = new QMenu(this);
    trayIconMenu -> addAction (minimizeAction);
    trayIconMenu -> addAction (restoreAction);
    trayIconMenu -> addAction (quitAction);
}

void MainWindow::showTrayIcon()
{
    // Создаём экземпляр класса и задаём его свойства...
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayImage(":/img/icon.png");
    trayIcon -> setIcon(trayImage);
    trayIcon -> setContextMenu(trayIconMenu);

    // Подключаем обработчик клика по иконке...
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    // Выводим значок...
    trayIcon -> show();
}
