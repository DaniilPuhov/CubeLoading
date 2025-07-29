#pragma once

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QTimer>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	explicit GLWidget(QWidget* parent = nullptr);
	enum Mode { Idle, Reading, Writing };

public slots:
	void setMode(Mode mode);
	void setModeToIdle();
	void setModeToReading();
	void setModeToWriting();

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

private:
	QTimer updateTimer;
	float angle = 300.0f; //Текущий угол наклона куба
	float aceleration = 0.1f; //Ускорение, применяемое к скорости куба
	float maxSpeed = 2.0f; //Максимальная скорость вращения куба
	float currentSpeed = 0.0f; //Текущая скорость куба
	Mode currentMode = Idle;
};
