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
    //Таймер до конца операции
    QElapsedTimer timer;
    timer.start();

    QString fullPath = QDir(targetDirectory).filePath(newFileName);
    QFileInfo fileInfo(targetDirectory);
    fullPath = fullPath + '.' + fileExtention;

    //Проверка файла на возможность создания
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit failed("Ошибка открития файла для записи: " + file.errorString());
        emit finished();
        return;
    }
   
    qint64 writtenBytes = 0; //Суммарный счётчик записанных байтов
    qint64 totalBytes = fileData.size(); //Размер файла в байтах
    const int chunkSize = 4069; //Размет записываемого за раз фрагмента файла
    qint64 lastBytes = 0; //Сколько данных было передано после предыдущего замера мгновенной скорости
    int lastUpdateTime = 0; //Счётчик времени в миллисекундах от начала работы таймера до прошлого измерения скорости 

    while (writtenBytes < totalBytes) {
        //Проверка отмены
        if (isStopped.load()) {
            file.close();
            file.remove();
            emit failed("Запись отменена");
            return;
        }
        //Проверка паузы
        QMutexLocker locker(&mutex);
        if (isPaused.load()) {
            waitCondition.wait(&mutex);
        }
        //Запись данных
        qint64 remaining = totalBytes - writtenBytes; //Сколько данных осталось записать
        qint64 bytesToWrite = qMin(static_cast<qint64>(chunkSize), remaining); //Сколько данных будет записано за следующую итерацию
        qint64 bytesWritten = file.write(fileData.constData() + writtenBytes, bytesToWrite); //Сколько данных было записано за прошедшую итерацию
        lastBytes += bytesWritten;

        if (bytesWritten <= 0) {
            emit failed("Запись прервана");
            file.close();
            file.remove();
            emit finished();
            return;
        }
        writtenBytes += bytesWritten;

        //Фиксация прогресса записи
        int percent = static_cast<int>((double(writtenBytes) / totalBytes) * 100.0);
        emit progress(percent);

        //Передача скорости и оставшегося времени
        int elapsed = timer.elapsed(); //Прошедшее время в миллсекундах от начала работы таймера
        int lastByteTime = elapsed - lastUpdateTime; //Сколько времени прошло с прошлого замера скорости

        if (elapsed - lastUpdateTime > 500) { //500 - частота обновления таймера
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