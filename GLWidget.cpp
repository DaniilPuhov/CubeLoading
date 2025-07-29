#include "GLWidget.h"
#include <QMatrix4x4>
#include <QtMath>

GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    connect(&updateTimer, &QTimer::timeout, this, [this]() {
        switch (currentMode)
        {
        case Writing:
            if (currentSpeed < maxSpeed) 
                currentSpeed += aceleration;
            angle += currentSpeed;
            break;
        case Reading:
            if (currentSpeed > -maxSpeed)
                currentSpeed -= aceleration;
            angle += currentSpeed;
            break;
        case Idle:
            if (currentSpeed > aceleration)
                currentSpeed -= aceleration;
            else if (currentSpeed < -aceleration)
                currentSpeed += aceleration;
            else if (currentSpeed > -aceleration && currentSpeed < aceleration)
                currentSpeed = 0;
            angle += currentSpeed;
            break;
        }
        update();
     });
    updateTimer.start(16); // �������� 60 FPS
}

void GLWidget::setMode(Mode mode) {
	currentMode = mode;
}

void GLWidget::setModeToIdle() {
    currentMode = Idle;
}

void GLWidget::setModeToReading() {
    currentMode = Reading;
}

void GLWidget::setModeToWriting() {
    currentMode = Writing;
}

void GLWidget::initializeGL() {
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = float(w) / float(h ? h : 1);
    glFrustum(-aspect, aspect, -1.0, 1.0, 2.0, 10.0);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f); // ������ �� ��� X
    glRotatef(30.0f, 0.0f, 1.0f, 0.0f); // ������ �� ��� Y
    glRotatef(angle, 0.0f, 0.0f, 1.0f); // ������ �� ��� Z

    glBegin(GL_QUADS);

    // �������� ����� (�������)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // ������ ����� (������)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    // ������� ����� (�����)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);

    // ������ ����� (Ƹ����)
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    // ������ ����� (�������)
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    // ����� ����� (�������)
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    glEnd();
}