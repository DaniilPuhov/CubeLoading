#include "WriteWorker.h"
#include <QFile>
#include <QThread>
#include <QDir>


WriteWorker::WriteWorker(const QString& directory, const QString& newName, const QByteArray& data, const QString& extention) : 
    targetDirectory(directory), newFileName(newName), fileData(data), fileExtention(extention), isPaused(false), isStopped(false) {}


void WriteWorker::pause() {
    isPaused.store(true);
}

void WriteWorker::resume() {
    if (isPaused.load() && !isStopped.load()) {
        isPaused.store(false);
        waitCondition.wakeOne();
    }
}

void WriteWorker::cancel() {
    isStopped.store(true);
    waitCondition.wakeAll();
}

void WriteWorker::process() {
    //������ �� ����� ��������
    QElapsedTimer timer;
    timer.start();

    QString fullPath = QDir(targetDirectory).filePath(newFileName);
    QFileInfo fileInfo(targetDirectory);
    fullPath = fullPath + '.' + fileExtention;

    //�������� ����� �� ����������� ��������
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit failed("������ �������� ����� ��� ������: " + file.errorString());
        emit finished();
        return;
    }
   
    qint64 writtenBytes = 0; //��������� ������� ���������� ������
    qint64 totalBytes = fileData.size(); //������ ����� � ������
    const int chunkSize = 4069; //������ ������������� �� ��� ��������� �����
    qint64 lastBytes = 0; //������� ������ ���� �������� ����� ����������� ������ ���������� ��������
    int lastUpdateTime = 0; //������� ������� � ������������� �� ������ ������ ������� �� �������� ��������� �������� 

    while (writtenBytes < totalBytes) {
        //�������� ������
        if (isStopped.load()) {
            file.close();
            file.remove();
            emit failed("������ ��������");
            return;
        }
        //�������� �����
        QMutexLocker locker(&mutex);
        if (isPaused.load()) {
            waitCondition.wait(&mutex);
        }
        //������ ������
        qint64 remaining = totalBytes - writtenBytes; //������� ������ �������� ��������
        qint64 bytesToWrite = qMin(static_cast<qint64>(chunkSize), remaining); //������� ������ ����� �������� �� ��������� ��������
        qint64 bytesWritten = file.write(fileData.constData() + writtenBytes, bytesToWrite); //������� ������ ���� �������� �� ��������� ��������
        lastBytes += bytesWritten;

        if (bytesWritten <= 0) {
            emit failed("������ ��������");
            file.close();
            file.remove();
            emit finished();
            return;
        }
        writtenBytes += bytesWritten;

        //�������� ��������� ������
        int percent = static_cast<int>((double(writtenBytes) / totalBytes) * 100.0);
        emit progress(percent);

        //�������� �������� � ����������� �������
        int elapsed = timer.elapsed(); //��������� ����� � ������������ �� ������ ������ �������
        int lastByteTime = elapsed - lastUpdateTime; //������� ������� ������ � �������� ������ ��������

        if (elapsed - lastUpdateTime > 500) { //500 - ������� ���������� �������
            double averageSpeed = (writtenBytes / 1024.0) / (elapsed / 1000.0);
            double instantSpeed = (lastBytes / 1024.0) / (lastByteTime / 1000.0);
            int timeRemaining = (averageSpeed > 0) ? static_cast<int>((totalBytes - writtenBytes) / 1024.0 / averageSpeed) : -1;

            emit speedAndETA(averageSpeed, instantSpeed, timeRemaining);
            lastUpdateTime = elapsed;
            lastBytes = 0;
        }
        QThread::msleep(10);
    }

    file.close();
    emit finished();
}