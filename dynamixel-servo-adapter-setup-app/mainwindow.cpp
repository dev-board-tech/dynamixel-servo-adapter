#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <about.h>

void MainWindow::comPortListRfsh() {
    QString t = ui->comboBox_Port->currentText();
    ui->comboBox_Port->clear();
    const QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info: infos ) {
        // Filter out virtual tty's
        if(!info.portName().contains("ttyS")) {
            ui->comboBox_Port->addItem(info.portName());
        }
    }
    ui->comboBox_Port->setCurrentText(t);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    comPortListRfsh();
    ui->comboBox_Port->installEventFilter(this);

    ui->groupBox_Action->setEnabled(false);

    f_ChangeId = new feature_ChangeId(this);
    f_ChangeId->setWindowModality(Qt::ApplicationModal);
    f_ChangeId->setWindowFlags( Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint );

    f_SetPosition = new feature_SetPosition(this);
    f_SetPosition->setWindowModality(Qt::ApplicationModal);
    f_SetPosition->setWindowFlags( Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint );

    ui->comboBox_Baud->addItems(QStringList(QList<QString>({"1200", "2400", "4800", "9600", "19200", "38400", "56800", "115200", "230400", "460800", "500000", "1000000"})));
    ui->comboBox_Baud->setCurrentText("1000000");

    listUsbTimer.setInterval(1000);
    listUsbTimer.start();
    serial = new QSerialPort();
    connect(&listUsbTimer, &QTimer::timeout, this, [this]() {
        bool equal = true;
        QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
        for (int i = 0; i < infos.size(); i++) {
            if(i < portListLast.size()) {
                if(infos[i].portName().compare(portListLast[i].portName())) {
                    equal = false;
                    break;
                }
            } else {
                equal = false;
                break;
            }
        }
        if(!equal) {
            portListLast = infos;
            comPortListRfsh();
        }
    });
    connect(ui->pushButton_Connect, &QPushButton::clicked, this, [this]() {
        if(!ui->pushButton_Connect->text().compare("Open")) {
            ui->pushButton_Connect->setText("Close");
            serial->setPortName(ui->comboBox_Port->currentText());
            serial->setBaudRate(ui->comboBox_Baud->currentText().toInt());
            serial->setDataBits(QSerialPort::Data8);
            serial->setParity(QSerialPort::NoParity);
            serial->setStopBits(QSerialPort::OneStop);
            //serial->setFlowControl(QSerialPort::NoFlowControl);
            if(serial->isOpen() || !serial->open(QIODevice::ReadWrite)) {
                ui->pushButton_Connect->setText("Open");
                QMessageBox messageBox;
                messageBox.critical(0,"Error","Can't open " + serial->portName() + ", error code: " + serial->errorString());
                return;
            }
            ui->plainTextEdit->clear();
            ui->comboBox_Port->setEnabled(false);
            ui->comboBox_Baud->setEnabled(false);

            ui->groupBox_Action->setEnabled(true);
            bool reversed[1] = {false};
            dxlInst = new dxl1x(serial, sizeof(reversed) / sizeof(reversed[0]), reversed);
        } else {
            delete dxlInst;
            ui->pushButton_Connect->setText("Open");
            serial->close();
            ui->comboBox_Port->setEnabled(true);
            ui->comboBox_Baud->setEnabled(true);

            ui->groupBox_Action->setEnabled(false);
        }
    });

    connect(ui->pushButton_Clear, &QPushButton::clicked, this, [this]() {
        ui->plainTextEdit->clear();
    });

    connect(ui->pushButton_Scan, &QPushButton::clicked, this, [this]() {
        ui->groupBox_Action->setEnabled(false);
        ui->groupBox_Action->repaint();
        ui->plainTextEdit->clear();
        for (int i = 1; i <= ui->spinBox_MaxServos->value(); i++) {
            dxl1x::DXL_ERR dxlErr;
            dxl1x::DXL_COMM_ERR err = dxlInst->ping(i, &dxlErr);
            ui->plainTextEdit->appendPlainText(QString::asprintf("%03d", i) + " -> " + dxlInst->showComError(err) + " -> " + dxlInst->showDxlError(dxlErr));
            ui->plainTextEdit->repaint();
        }
        ui->groupBox_Action->setEnabled(true);
    });

    connect(ui->pushButton_ReadParam, &QPushButton::clicked, this, [this]() {
        ui->plainTextEdit->clear();
        dxl1x::DXL_COMM_ERR err;
        dxl1x::DXL_DATA_t valueTable;
        dxl1x::DXL_ERR dxlErr;
        err = dxlInst->read((unsigned char *)&valueTable , ui->spinBox_ServoNr->value() , 0, sizeof(dxl1x::DXL_DATA_t), &dxlErr);
        ui->plainTextEdit->appendPlainText(QString::asprintf("%03d", ui->spinBox_ServoNr->value()) + " -> " + dxlInst->showComError(err) + " -> " + dxlInst->showDxlError(dxlErr));
        ui->plainTextEdit->appendPlainText(dxlInst->toHumanReadable(&valueTable));
    });

    connect(ui->pushButton_ChangeId, &QPushButton::clicked, this, [this]() {
        f_ChangeId->setDxlInst(dxlInst);
        f_ChangeId->show();
    });

    connect(ui->pushButton_SetPos, &QPushButton::clicked, this, [this]() {
        f_SetPosition->setDxlInst(dxlInst);
        f_SetPosition->setId(ui->spinBox_ServoNr->value());
        f_SetPosition->show();
    });

    connect(ui->pushButton_About, &QPushButton::clicked, this, []() {
        about a;
        a.exec();
    });

}

MainWindow::~MainWindow() {
    delete f_ChangeId;
    delete ui;
}

bool MainWindow::eventFilter( QObject *o, QEvent *e ) {
    if (o == ui->comboBox_Port) {
        if(e->type() == QEvent::MouseButtonPress) {
            comPortListRfsh();
        }
    } else {
        // standard event processing
        return false;
    }
    return QMainWindow::eventFilter(o, e);
}
