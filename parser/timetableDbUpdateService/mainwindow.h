#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_inacsrv.h"
#include <QMainWindow>
#include <QUrl>
#include <QFile>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSystemTrayIcon>
#include <map>

class myDlg:public QDialog
{
public:
    Ui::Dialog *ui;
    myDlg(QWidget *p):
    QDialog(p),
    ui(new Ui::Dialog)
    {
        ui->setupUi(this);
        setModal(true);
    }
    ~myDlg(){delete ui;}
};
namespace Ui {
class MainWindow;
}
//дерево выриантов выбора на сайте
struct Tree{
    //тип - выбор
    struct item{
        //кол-во потомков (children),
        //порядковый номер текущего экземпляра среди всех потомков родителя (id),
        //кол-во потомков чьи дочерние ветви полностью построены (done)
        int children, id, done;
        //номер (индекс) родителя в веаторе дерева
        int parent;
        //имя параметра (name)
        //таблица (table)
        //id в таблице (tId)
        //имя в таблице (tName)
        QString name, table, tId, tVal;
        //значение параметра и содержимое тега
        std::vector<QString> *values, *contain;
        //конструктор по умолчанию
        item()
        {
            //qDebug() << "item. constructor";
            values = new std::vector<QString>;
            contain = new std::vector<QString>;
            done = children = 0;
            table = tId = "";
        }
        //конструктор копирования
        item(const item& a)
        {
            //qDebug() << "item. copy constructor";
            values = new std::vector<QString>;
            *values = *a.values;
            contain = new std::vector<QString>;
            *contain = *a.contain;
            children = a.children;
            parent = a.parent;
            name = a.name;
            id = a.id;
            done = a.done;
            table = a.table;
            tId = a.tId;
            tVal = a.tVal;
        }
        ~item()
        {
            //qDebug() << "item. destructor";
            delete values;
            delete contain;
        }
    };
    //хранилище вариантов
    std::vector<item> *items;
    Tree()
    {
        items = new std::vector<item>;
    }
    ~Tree()
    {
        delete items;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool needShow;
private slots:
    //получен результат запроса к серверу
    void replyFinished(QNetworkReply *reply);
    //загрузка данных завершена
    void onLoadDone();
    //построение дерева вариантов завершено
    void onAllDone();
    void on_abort_clicked();
    void onServerInaccessible();
    void onRepeat();
    void onTimeout();
    void on_toBackgraund_clicked();
    void on_info_clicked();
    void onInfoDlgQuit();
    void changeEvent(QEvent* event);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void trayActionExecute();
    void setTrayIconActions();
    void showTrayIcon();

signals:
    void loadDone();
    void allDone();
private:
    //выполнить запрос
    void doDownload(const char *url, const char *param);
    //разобрать строку ответа
    int parse(int prnt);
    QString getPhrse(const QString &source, const QString &beginsOn,
                     const QString &endsOn, int &index);
    void completeTheBranch(const QString &str, int prnt);
    void readTheTable(const QString &str, int prnt);
    QString putToBase(const QString &table, const QString &value);
    QString putDayToBase(const QString &day, const QString &date, const QString &week);
    void putPairToBase(const QString &form, const QString &fak,
                       const QString &kurs, const QString &group,
                       const QString &interval, const QString &day,
                       const QString &time,  const QString &obj,
                       const QString &room, const QString &teacher);
    QString dateFormat(const QString &date);
    void loadDefinitions();
    void clearBase();
    Ui::MainWindow *ui;
    //мененджер доступа к сети
    QNetworkAccessManager *manager;
    //буфер ответа
    QByteArray buf;
    //кодек для перевода кодировок
    QTextCodec *codec;
    //дерево вариантов (меню) сайта
    Tree *tree;
    //аддресс страницы сайта
    QString myUrl;
    QString address, params;
    myDlg *dlg;
    QTimer *timer;
    QNetworkReply *currentReply;
    QSqlDatabase db;
    typedef std::map<QString, QString> Definitions;
    typedef std::pair<QString, QString> DsPair;
    Definitions *definitions;

    QMenu *trayIconMenu;
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
};

#endif // MAINWINDOW_H
