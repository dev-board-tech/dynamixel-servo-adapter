#include "feature_setposition.h"
#include "ui_feature_setposition.h"

#include <QDial>
#include <QMessageBox>

feature_SetPosition::feature_SetPosition(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::feature_SetPosition) {
    ui->setupUi(this);

    connect(ui->dial, &QDial::valueChanged, this, [this](int value) {
        unsigned short v = value;
        dxl1x::DXL_ERR dxlErr;
        dxl1x::DXL_COMM_ERR err = dxlInst->setGoalPosition(id, v, &dxlErr);
    });
}

feature_SetPosition::~feature_SetPosition() {
    delete ui;
}

void feature_SetPosition::setId(int id) {
    this->id = id;
    this->setWindowTitle("ID: " + QString::number(id));
    dxl1x::DXL_COMM_ERR err;
    unsigned short value;
    dxl1x::DXL_ERR dxlErr;
    err = dxlInst->read((unsigned char *)&value , id , dxl1x::DXL_PRESENT_POSITION_L, 2, &dxlErr);
    if(err != dxl1x::DXL_COMM_ERR::DXL_COMM_SUCCESS || dxlErr != dxl1x::DXL_ERR::DXL_ERR_OK) {
        QMessageBox::critical(
            this,
            tr("Change ID"),
            (QString::asprintf("%03d", id) + " -> " + dxlInst->showComError(err) + " -> " + dxlInst->showDxlError(dxlErr))
            );
    } else {
        ui->dial->setValue(value);
    }
}

void feature_SetPosition::setDxlInst(dxl1x *dxlInst) {
    this->dxlInst = dxlInst;
}
