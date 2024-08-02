#include "about.h"
#include "ui_about.h"
#include <QDialogButtonBox>

about::about(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::about)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        this->close();
    });
}

about::~about()
{
    delete ui;
}
