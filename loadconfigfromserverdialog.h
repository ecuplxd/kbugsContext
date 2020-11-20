#ifndef LOADCONFIGFROMSERVERDIALOG_H
#define LOADCONFIGFROMSERVERDIALOG_H

#include <QDialog>

namespace Ui {
class LoadConfigFromServerDialog;
}

class LoadConfigFromServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadConfigFromServerDialog(QWidget *parent = nullptr);
    ~LoadConfigFromServerDialog();

private:
    Ui::LoadConfigFromServerDialog *ui;
};

#endif // LOADCONFIGFROMSERVERDIALOG_H
