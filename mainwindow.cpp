#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTime>

MainWindow::MainWindow(QWidget* parent) //Основной класс программы
    : QMainWindow(parent), readWorker(nullptr), readWorkerThread(nullptr), writeWorker(nullptr), writeWorkerThread(nullptr), glWidget(nullptr) {
    
    //Создание кнопок, полей, графики
    auto* central = new QWidget;
    auto* layout = new QVBoxLayout;
    auto* buttonLayout = new QHBoxLayout;
    
    fileNameEdit = new QLineEdit(this);
    fileNameEdit->setPlaceholderText("Введите новое имя файла");
    fileNameEdit->setFixedWidth(400);
    
    loadButton = new QPushButton("Прочитать файл...");
    loadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    saveButton = new QPushButton("Сохранить как...");
    saveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pauseButton = new QPushButton("Пауза");
    pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    resumeButton = new QPushButton("Возобновить");
    resumeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cancelButton = new QPushButton("Отмена");
    cancelButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    progressBar = new QProgressBar;
    progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    progressBar->setFixedWidth(435);
    averageSpeedLabel = new QLabel("Средняя скорость: 0 КБ/с", this);
    averageSpeedLabel->setAlignment(Qt::AlignLeft);
    instantSpeedLabel = new QLabel("Мгновенная скорость: 0 КБ/с", this);
    instantSpeedLabel->setAlignment(Qt::AlignLeft);
    etaLabel = new QLabel("Осталось времени: 0 сек.", this);
    etaLabel->setAlignment(Qt::AlignLeft);

    glWidget = new GLWidget(this);
    glWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);

    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(pauseButton);
    buttonLayout->addWidget(resumeButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
    layout->addWidget(progressBar);
    layout->addWidget(fileNameEdit);
    layout->addWidget(averageSpeedLabel);
    layout->addWidget(instantSpeedLabel);
    layout->addWidget(etaLabel);
    layout->addWidget(glWidget);
    layout->addWidget(logTextEdit);

    central->setLayout(layout);
    setCentralWidget(central);

    //Подключение кнопок, полей
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::startReading);
    connect(loadButton, &QPushButton::clicked, glWidget, &GLWidget::setModeToReading);
    connect(loadButton, &QPushButton::clicked, this, [this]() {
        logMessage("Чтение начато"); //Передача сообщения в лог
        });
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::pause);
    connect(pauseButton, &QPushButton::clicked, glWidget, &GLWidget::setModeToIdle);
    connect(pauseButton, &QPushButton::clicked, this, [this]() {
        if (readWorker && readWorkerThread && readWorkerThread->isRunning()) {
            logMessage("Чтение приостановлено");
        }
        if (writeWorker && writeWorkerThread && writeWorkerThread->isRunning()) {
            logMessage("Запись приостановлена");
        }
        });
    connect(resumeButton, &QPushButton::clicked, this, &MainWindow::resume);
    connect(resumeButton, &QPushButton::clicked, this, [this]() {
        if (readWorker && readWorkerThread && readWorkerThread->isRunning()) {
            logMessage("Чтение возобновлено");
        }
        if (writeWorker && writeWorkerThread && writeWorkerThread->isRunning()) {
            logMessage("Запись возобновлена");
        }
        });
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::startSaving);
    connect(saveButton, &QPushButton::clicked, glWidget, &GLWidget::setModeToWriting);
    connect(saveButton, &QPushButton::clicked, this, [this]() {
        logMessage("Запись начата");
        });
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::cancel);
    connect(cancelButton, &QPushButton::clicked, glWidget, &GLWidget::setModeToIdle);
    connect(cancelButton, &QPushButton::clicked, this, [this]() {
        if (readWorker && readWorkerThread && readWorkerThread->isRunning()) {
            logMessage("Чтение отменено");
        }
        if (writeWorker && writeWorkerThread && writeWorkerThread->isRunning()) {
            logMessage("Запись отменена");
        }
        });
    glWidget->setMinimumSize(200, 200);
    glWidget->show();
}

MainWindow::~MainWindow() {
    if (readWorkerThread && readWorkerThread->isRunning()) {
        readWorkerThread->quit();
        readWorkerThread->wait();
        readWorkerThread->deleteLater();
    }
    if (writeWorkerThread && writeWorkerThread->isRunning()) {
        writeWorkerThread->quit();
        writeWorkerThread->wait();
        writeWorkerThread->deleteLater();
    }
    
}

void MainWindow::startReading() {
    originalFilePath = QFileDialog::getOpenFileName(this, "Open File");
    if (originalFilePath.isEmpty()) return;

    readWorker = new ReadWorker(originalFilePath);
    readWorkerThread = new QThread;

    readWorker->moveToThread(readWorkerThread);

    connect(readWorkerThread, &QThread::started, readWorker, &ReadWorker::process);
    connect(readWorker, &ReadWorker::progress, this, &MainWindow::updateProgress);
    connect(readWorker, &ReadWorker::finished, this, &MainWindow::readingFinished);
    connect(readWorker, &ReadWorker::finished, readWorkerThread, &QThread::quit);
    connect(readWorker, &ReadWorker::finished, glWidget, &GLWidget::setModeToIdle);
    connect(readWorker, &ReadWorker::finished, this, [this]() {
        logMessage("Чтение завершено");
        });
    connect(readWorkerThread, &QThread::finished, readWorker, &QObject::deleteLater);
    connect(readWorker, &ReadWorker::speedAndETA, this, &MainWindow::onSpeedAndETA);
    connect(readWorker, &ReadWorker::failed, this, &MainWindow::failed);

    readWorkerThread->start();
}

void MainWindow::pause() {
    if (readWorker) readWorker->pause();
    if (writeWorker) writeWorker->pause();
}

void MainWindow::resume() {
    if (readWorker) {
        readWorker->resume();
        glWidget->setModeToReading();
    }
    if (writeWorker) {
        writeWorker->resume();
        glWidget->setModeToWriting();
    }
}

void MainWindow::updateProgress(int percent) {
    progressBar->setValue(percent);
}

void MainWindow::readingFinished(const QByteArray &data) {  //Сохранение пути и данных входного файла
    QFileInfo fileinfo(originalFilePath);
    fileNameEdit->setPlaceholderText(fileinfo.fileName());
    progressBar->setValue(100);
    fileData = data;
}

void MainWindow::onSpeedAndETA(double averageSpeed, double instantSpeed, int secondsRemaining) {
    averageSpeedLabel->setText(QString("Средняя скорость: %1 КБ/с").arg(averageSpeed, 0, 'f', 2));
    instantSpeedLabel->setText(QString("Мгновенная скорость: %1 КБ/с").arg(instantSpeed, 0, 'f', 2));

    QTime etaTime = QTime(0, 0).addSecs(secondsRemaining);
    etaLabel->setText(QString("Осталось времени: %1").arg(etaTime.toString("mm:ss")));
}

void MainWindow::startSaving() {
    QString dir = QFileInfo(originalFilePath).absolutePath();
    QString newName = fileNameEdit->text().trimmed();
    QString extention = QFileInfo(originalFilePath).suffix();

    writeWorker = new WriteWorker(dir, newName, fileData, extention);
    writeWorkerThread = new QThread;
    writeWorker->moveToThread(writeWorkerThread);

    connect(writeWorkerThread, &QThread::started, writeWorker, &WriteWorker::process);
    connect(writeWorker, &WriteWorker::progress, this, &MainWindow::updateProgress);
    connect(writeWorker, &WriteWorker::finished, writeWorkerThread, &QThread::quit);
    connect(writeWorkerThread, &QThread::finished, writeWorker, &QObject::deleteLater);
    connect(writeWorker, &WriteWorker::finished, glWidget, &GLWidget::setModeToIdle);
    connect(writeWorker, &WriteWorker::finished, this, [this]() {
        logMessage("Запись завершена");
        });
    connect(writeWorker, &WriteWorker::speedAndETA, this, &MainWindow::onSpeedAndETA);
    connect(writeWorker, &WriteWorker::failed, this, &MainWindow::failed);

    writeWorkerThread->start();
}

void MainWindow::cancel() {
    if (readWorker && readWorkerThread && readWorkerThread->isRunning()) {
        readWorker->cancel();
    }
    if (writeWorker && writeWorkerThread && writeWorkerThread->isRunning()) {
        writeWorker->cancel();
    }
}

void MainWindow::failed() {
    if (readWorkerThread) {
        readWorkerThread->quit();
        readWorkerThread->wait();
        readWorker = nullptr;
        readWorkerThread->deleteLater();
        readWorkerThread = nullptr;
    }
    if (writeWorkerThread) {
        writeWorkerThread->quit();
        writeWorkerThread->wait();
        writeWorker = nullptr;
        writeWorkerThread->deleteLater();
        writeWorkerThread = nullptr;
    }
}

void MainWindow::logMessage(const QString& message) {
    QString timestamp = QTime::currentTime().toString("HH:mm:ss");
    logTextEdit->append("[" + timestamp + "] " + message);
}