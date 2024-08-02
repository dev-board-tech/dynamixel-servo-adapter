#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QtSerialPort/QSerialPort"
#include <QSerialPortInfo>
#include <QTimer>

#include "dxl1x.h"
#include "feature_changeid.h"
#include "feature_setposition.h"



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;

    QTimer listUsbTimer;

    QByteArray receiveArray;
    QByteArray dataToSend;

    dxl1x *dxlInst;
    QList<QSerialPortInfo> portListLast;

    feature_ChangeId *f_ChangeId;
    feature_SetPosition *f_SetPosition;

    void comPortListRfsh();
protected:
    bool eventFilter( QObject *o, QEvent *e );
};
#endif // MAINWINDOW_H
