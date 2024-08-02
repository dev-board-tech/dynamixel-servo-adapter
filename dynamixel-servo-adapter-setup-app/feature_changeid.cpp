#include "feature_changeid.h"
#include "ui_feature_changeid.h"

#include <QMessageBox>

feature_ChangeId::feature_ChangeId(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::feature_ChangeId) {
    ui->setupUi(this);

    connect(ui->pushButton_Apply, &QPushButton::clicked, this, [this]() {
        dxl1x::DXL_COMM_ERR err;
        dxl1x::DXL_ERR dxlErr;
        err = dxlInst->changeId(ui->spinBox_CurrentId->value() , ui->spinBox_NewId->value(), &dxlErr);
        if(err != dxl1x::DXL_COMM_ERR::DXL_COMM_SUCCESS || dxlErr != dxl1x::DXL_ERR::DXL_ERR_OK) {
            QMessageBox::critical(
                this,
                tr("Change ID"),
                (QString::asprintf("%03d", ui->spinBox_CurrentId->value()) + " -> " + dxlInst->showComError(err) + " -> " + dxlInst->showDxlError(dxlErr))
                );
        } else {
            QMessageBox::information(
                this,
                tr("Change ID"),
                tr("New ID is: " + QString::number(ui->spinBox_NewId->value()).toLocal8Bit())
                );
        }
    });
}

feature_ChangeId::~feature_ChangeId() {
    delete ui;
}

void feature_ChangeId::setDxlInst(dxl1x *dxlInst) {
    this->dxlInst = dxlInst;
}
