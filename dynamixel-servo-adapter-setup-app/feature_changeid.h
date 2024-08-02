#ifndef FEATURE_CHANGEID_H
#define FEATURE_CHANGEID_H

#include <QMainWindow>
#include "dxl1x.h"

namespace Ui {
class feature_ChangeId;
}

class feature_ChangeId : public QMainWindow
{
    Q_OBJECT

public:
    explicit feature_ChangeId(QWidget *parent = nullptr);
    ~feature_ChangeId();

    void setDxlInst(dxl1x *dxlInst);
private:
    Ui::feature_ChangeId *ui;

    dxl1x *dxlInst;
};

#endif // FEATURE_CHANGEID_H
