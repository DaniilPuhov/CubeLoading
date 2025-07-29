#include "ReadWorker.h"
#include <QFile>
#include <QTextStream>
#include <QThread>

ReadWorker::ReadWorker(const QString& filePath) : filePath(filePath), isPaused(false), isStopped(false) {}

void ReadWorker::pause() {
    isPaused.store(true);
}

void ReadWorker::resume() {
    if (isPaused.load() && !isStopped.load()) {
        isPaused.store(false);
        waitCondition.wakeOne();
    }
}

void ReadWorker::cancel() {
    isStopped.store(true);
    waitCondition.wakeAll();
}

void ReadWorker::process() {
    //������ �� ����� ��������
    QElapsedTimer timer;
    timer.start();

    //�������� �����
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit failed("������ �������� �����");
        return;
    }
    fileData.clear();

    qint64 totalBytes = file.size(); //������ ����� � ������
    qint64 readBytes = 0;            //��������� ������� ����������� ������
    const int chunkSize = 4096; //������ ������������ �� ��� ��������� �����
    qint64 lastBytes = 0; //������� ������ ���� �������� ����� ����������� ������ ���������� ��������
    int lastUpdateTime = 0; //������� ������� � ������������� �� ������ ������ ������� �� �������� ��������� �������� 

    while (!file.atEnd()) {
        //�����
        {
            QMutexLocker locker(&mutex);
            if (isPaused.load()) {
                waitCondition.wait(&mutex);
            }
        }

        //������ ������
        if (isStopped.load()) {
            emit failed("�������� �������������");
            return;
        }

        //������
        QByteArray chunk = file.read(chunkSize);
        if (chunk.isEmpty()) break;
        fileData.append(chunk);
        readBytes += chunk.size();
        lastBytes += chunk.size();

        //�������� ��������� ������
        int percent = static_cast<int>((double(readBytes) / totalBytes) * 100.0);
        emit progress(percent);

        //�������� �������� � ����������� �������
        int elapsed = timer.elapsed(); //��������� ����� � ������������� �� ������ ������ �������
        int lastByteTime = elapsed - lastUpdateTime; //������� ������� ������ � �������� ������ ��������

        if (elapsed - lastUpdateTime > 500) { //500 - �������� ����� ������������ ������� � �������������
            double averageSpeed = (readBytes / 1024.0) / (elapsed / 1000.0);
            double instantSpeed = (lastBytes / 1024.0) / (lastByteTime / 1000.0);
            int timeRemaining = (averageSpeed > 0) ? static_cast<int>((totalBytes - readBytes) / 1024.0 / averageSpeed) : -1;
            
            emit speedAndETA(averageSpeed, instantSpeed, timeRemaining);
            lastUpdateTime = elapsed;
            lastBytes = 0; 
        }

        QThread::msleep(10);  //�������� ��� ������������ ��������
    }

    isPaused.store(true);
    file.close();

    emit finished(fileData);
};