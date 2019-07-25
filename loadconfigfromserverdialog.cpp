#include "loadconfigfromserverdialog.h"
#include "ui_loadconfigfromserverdialog.h"

LoadConfigFromServerDialog::LoadConfigFromServerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadConfigFromServerDialog)
{
    ui->setupUi(this);
}

LoadConfigFromServerDialog::~LoadConfigFromServerDialog()
{
    delete ui;
}
