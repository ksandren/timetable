#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include <QWidget>

namespace Ui {
class AboutDlg;
}

class AboutDlg : public QWidget
{
    Q_OBJECT
public:
    explicit AboutDlg(QWidget *parent = 0);

signals:

public slots:
private:
    Ui::AboutDlg *ui;
};

#endif // ABOUTDLG_H
