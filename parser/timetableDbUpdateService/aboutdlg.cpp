#include "aboutdlg.h"
#include "ui_about.h"

AboutDlg::AboutDlg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutDlg)
{
    ui->setUi(this);
}

