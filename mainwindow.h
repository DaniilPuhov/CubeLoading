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
	void readingFinished(const QByteArray& data); //��������� ����������� ���� � ���� fileData
	void onSpeedAndETA(double averageSpeed, double instantSpeed, int secondsRemaining); //������� �������� ������/������ � ���������� �����
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

	QByteArray fileData; //����������� ����

	QLineEdit* fileNameEdit;
	QString originalFilePath;

	QLabel* averageSpeedLabel; //������� �������� ������/����������
	QLabel* instantSpeedLabel; //������� �������� ������/����������
	QLabel* etaLabel; //���������� ����� ������/����������

	ReadWorker* readWorker;
	QThread* readWorkerThread;

	WriteWorker* writeWorker;
	QThread* writeWorkerThread;

	GLWidget* glWidget; //���
	QTextEdit* logTextEdit; //���. �� ������������ � ��������� ���� � ���������� ������ �� ����� ���������� ���������
	void logMessage(const QString& message);
};
