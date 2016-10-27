#ifndef LOADER_H
#define LOADER_H

#include <QObject>
#include <QUrl>
#include <QFile>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QIODevice>
#include <QByteArray>
#include <QTextCodec>
#include <QTextStream>

struct Tree{
    struct item{
        QString name;
        std::vector<QString> values, contain;
        std::vector<item *> children;
    };
    std::vector<item> items;
};

class Loader: public QObject
{
    Q_OBJECT
private:
    Tree *tree;
    QNetworkAccessManager *manager;
    QTextCodec *codec;
    QObject *parent;
public:
    void doDownload(const char *url, const char *param);
    Loader(QObject *parent_, Tree *tree_, QTextCodec *codec_);
    ~Loader();
signals:
    void done();
public slots:
    void replyFinished(QNetworkReply *reply);
};

#endif // LOADER_H
