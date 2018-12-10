#ifndef DIALOGCOMMANDSET_H
#define DIALOGCOMMANDSET_H

#include <QDialog>
#include <c_config.h>
namespace Ui {
class DialogCommandSet;
}

class DialogCommandSet : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCommandSet(QWidget *parent = 0);
    ~DialogCommandSet();

private slots:
    void on_buttonBox_accepted();

    void on_pushButton_save_clicked();

private:
    Ui::DialogCommandSet *ui;
};

#endif // DIALOGCOMMANDSET_H
