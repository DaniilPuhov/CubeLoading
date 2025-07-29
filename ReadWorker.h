#pragma once

#include <QObject>
#include <QString>
#include <QAtomicInt>
#include <QMutex>
#include <QWaitCondition>

class ReadWorker : public QObject {
    Q_OBJECT

public:
    explicit ReadWorker(const QString& filePath);
    void pause();
    void resume();
    void cancel();
signals:
    void progress(int percent);
    void speedAndETA(double averageSpeed, double instantSpeed, int secondsRemaining); //Передача средней скорости, мгновенной скорости и оставшегося времени (Estimated Time of Arrival)
    void finished(const QByteArray &data);
    void failed(QString message);

public slots:
    void process();

private:
    QString filePath;
    QAtomicInt isPaused;
    QAtomicInt isStopped;
    QMutex mutex;
    QWaitCondition waitCondition;
    QByteArray fileData; //Данные загруженного файла
};