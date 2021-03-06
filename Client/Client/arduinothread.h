#ifndef ARDUINOTHREAD_H
#define ARDUINOTHREAD_H

#include <QtCore>
#include <QThread>
#include "arduinomessagebean.h"
#include "Communication.h"
#include "sim3dthread.h"


class ArduinoThread : public QThread
{
    Q_OBJECT
public:
    explicit ArduinoThread(QObject* parent=0);
    void run();
    bool isAlive();
    void terminate();
    bool sim3dStarted = false;
    ArduinoMessageBean msg;
    QMutex *mtx;
    Sim3DThread *simThread;
    int mode = 1;
signals:
    void readySend(int,int,int,int);
    void startArdThread();
    void updateServoPlotDataArd(int mx,int my);
    void updateXYPlotDataArd(int bx,int by);
public slots:
    void updateXPID(int,int,int);
    void updateYPID(int,int,int);
    void ChangeMode(int);
    void Started();
    void HandleRequest(int mode);
private:
    void takeCoinInfo(int* first,int* second,int* third);
    Communication *com = NULL;
    bool alive;
    bool connectionCompleted;
};
#endif // ARDUINOTHREAD_H
