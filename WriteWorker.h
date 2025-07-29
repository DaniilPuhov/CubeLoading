#pragma once

#include <QObject>
#include <QString>
#include <QAtomicInt>
#include <QMutex>
#include <QWaitCondition>

class WriteWorker : public QObject
{
	Q_OBJECT

public:
	explicit WriteWorker(const QString& directory, const QString& newName, const QByteArray& data, const QString& extention);
	void pause();
	void resume();
	void cancel();
signals:
	void progress(int percent);
	void speedAndETA(double averageSpeed, double instantSpeed, int secondsRemaining); //Передача скорости и оставшегося времени (Estimated Time of Arrival)
	void finished();
	void failed(QString message);

public slots:
	void process();

private:
	QString targetDirectory;
	QString newFileName;
	QAtomicInt isPaused;
	QAtomicInt isStopped;
	QMutex mutex;
	QWaitCondition waitCondition;
	QByteArray fileData; //Данные загруженного файла
	QString fileExtention;
};
