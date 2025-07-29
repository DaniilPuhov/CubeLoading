#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QProgressBar>
#include <QThread>
#include <QLineEdit>
#include "ReadWorker.h"
#include "WriteWorker.h"
#include <QLabel>
#include "GLWidget.h"
#include <QTextEdit>

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();
private slots:
	void startReading();
	void pause();
	void resume();
	void readingFinished(const QByteArray& data); //Сохраняет прочитанный файл в поле fileData
	void onSpeedAndETA(double averageSpeed, double instantSpeed, int secondsRemaining); //Считает скорость чтения/записи и оставшееся время
	void cancel();
	void failed();
	void updateProgress(int percent);
	void startSaving();

private:
	QPushButton* loadButton;
	QPushButton* pauseButton;
	QPushButton* resumeButton;
	QProgressBar* progressBar;
	QPushButton* saveButton;
	QPushButton* cancelButton;

	QByteArray fileData; //Загруженный файл

	QLineEdit* fileNameEdit;
	QString originalFilePath;

	QLabel* averageSpeedLabel; //Средняя скорость чтения/сохранения
	QLabel* instantSpeedLabel; //Текущая скорость чтения/сохранения
	QLabel* etaLabel; //Оставшееся время чтения/сохранения

	ReadWorker* readWorker;
	QThread* readWorkerThread;

	WriteWorker* writeWorker;
	QThread* writeWorkerThread;

	GLWidget* glWidget; //Куб
	QTextEdit* logTextEdit; //Лог. Не записывается в отдельный файл и существует только во время выполнения программы
	void logMessage(const QString& message);
};
