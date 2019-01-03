#ifndef DIALOGCONFIG_H
#define DIALOGCONFIG_H

#include <QDialog>
#include "c_config.h"
namespace Ui {
class DialogConfig;
}

class DialogConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConfig(QWidget *parent = 0);
    ~DialogConfig();

private slots:
    void on_pushButton_load_clicked();

private:
    Ui::DialogConfig *ui;
};

#endif // DIALOGCONFIG_H
