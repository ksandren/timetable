#include "loader.h"

Loader::Loader(QObject *parent_, Tree *tree_, QTextCodec *codec_):
    parent(parent_), tree(tree_), codec(codec_)
{
}
Loader::~Loader(){
    delete manager;
}
void Loader::doDownload(const char *url, const char *param)
{
    manager = new QNetworkAccessManager(parent);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    manager->post(QNetworkRequest(QUrl(url)), QByteArray(param));
}
void Loader::replyFinished (QNetworkReply *reply)
{
   if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
    }
    else
    {
        qDebug() << reply->header(QNetworkRequest::ContentTypeHeader).toString();
        qDebug() << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString();
        qDebug() << reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
        qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString isOk = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        qDebug() << isOk;
        if(isOk.indexOf("OK") != -1)
        {
            QByteArray msg = reply->readAll();
            QString strf = codec->toUnicode(msg);
            int index = strf.indexOf("<select");
            QString key = "</select>";
            int selEnd = strf.indexOf(key, index);
            key = "name=\"";
            index = strf.indexOf(key, index);
            index = index + key.length();
            int len = strf.indexOf("\"", index) - index;
            QString name = strf.mid(index, len);
            QString value, contain;
            key = "<option value=\"";
            index = strf.indexOf(key, index);
            Tree::item it;
            it.name = name;
            while(index != -1 && index < selEnd)
            {
                index = index + key.length();
                len = strf.indexOf("\"", index) - index;
                value = strf.mid(index, len);
                index += len;
                key = ">";
                index = strf.indexOf(key, index);
                index = index + key.length();
                len = strf.indexOf("<", index) - index;
                contain = strf.mid(index, len);
                key = "<option value=\"";
                index = strf.indexOf(key, index);
                it.values.push_back(value);
                it.contain.push_back(contain);
            }
            tree->items.push_back(it);
            qDebug() << value << " " << contain;
        }
    }
    reply->deleteLater();
    emit done();
    //close();
}

