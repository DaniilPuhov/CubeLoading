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
    //Таймер до конца операции
    QElapsedTimer timer;
    timer.start();

    //Открытие файла
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit failed("Ошибка открытия файла");
        return;
    }
    fileData.clear();

    qint64 totalBytes = file.size(); //Размер файла в байтах
    qint64 readBytes = 0;            //Суммарный счётчик прочитанных байтов
    const int chunkSize = 4096; //Размет считываемого за раз фрагмента файла
    qint64 lastBytes = 0; //Сколько данных было передано после предыдущего замера мгновенной скорости
    int lastUpdateTime = 0; //Счётчик времени в миллисекундах от начала работы таймера до прошлого измерения скорости 

    while (!file.atEnd()) {
        //Пауза
        {
            QMutexLocker locker(&mutex);
            if (isPaused.load()) {
                waitCondition.wait(&mutex);
            }
        }

        //Отмена чтения
        if (isStopped.load()) {
            emit failed("Отменено пользователем");
            return;
        }

        //Чтение
        QByteArray chunk = file.read(chunkSize);
        if (chunk.isEmpty()) break;
        fileData.append(chunk);
        readBytes += chunk.size();
        lastBytes += chunk.size();

        //Фиксация прогресса чтения
        int percent = static_cast<int>((double(readBytes) / totalBytes) * 100.0);
        emit progress(percent);

        //Передача скорости и оставшегося времени
        int elapsed = timer.elapsed(); //Прошедшее время в миллисекундах от начала работы таймера
        int lastByteTime = elapsed - lastUpdateTime; //Сколько времени прошло с прошлого замера скорости

        if (elapsed - lastUpdateTime > 500) { //500 - задержка между обновлениями таймера в миллисекундах
            double averageSpeed = (readBytes / 1024.0) / (elapsed / 1000.0);
            double instantSpeed = (lastBytes / 1024.0) / (lastByteTime / 1000.0);
            int timeRemaining = (averageSpeed > 0) ? static_cast<int>((totalBytes - readBytes) / 1024.0 / averageSpeed) : -1;
            
            emit speedAndETA(averageSpeed, instantSpeed, timeRemaining);
            lastUpdateTime = elapsed;
            lastBytes = 0; 
        }

        QThread::msleep(10);  //Задержка для демонстрации загрузки
    }

    isPaused.store(true);
    file.close();

    emit finished(fileData);
};