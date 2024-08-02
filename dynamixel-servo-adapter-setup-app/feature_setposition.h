#ifndef FEATURE_SETPOSITION_H
#define FEATURE_SETPOSITION_H

#include <QMainWindow>
#include "dxl1x.h"

namespace Ui {
class feature_SetPosition;
}

class feature_SetPosition : public QMainWindow
{
    Q_OBJECT

public:
    explicit feature_SetPosition(QWidget *parent = nullptr);
    ~feature_SetPosition();
    void setId(int id);
    void setDxlInst(dxl1x *dxlInst);

private:
    Ui::feature_SetPosition *ui;

    dxl1x *dxlInst;
    int id;
    int goalPosition;
};

#endif // FEATURE_SETPOSITION_H
