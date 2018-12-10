#ifndef DIALOGMENUDISPLAY_H
#define DIALOGMENUDISPLAY_H

#include <QDialog>

namespace Ui {
class DialogMenuDisplay;
}

class DialogMenuDisplay : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMenuDisplay(QWidget *parent = 0);
    ~DialogMenuDisplay();

private:
    Ui::DialogMenuDisplay *ui;
};

#endif // DIALOGMENUDISPLAY_H
