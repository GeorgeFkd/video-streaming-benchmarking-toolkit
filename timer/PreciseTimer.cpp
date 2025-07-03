#include "PreciseTimer.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QPoint>
#include <QTimer>
#include <QVBoxLayout>
#include <QtWidgets>
#include <QDateTime>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <qnamespace.h>
PreciseTimer::PreciseTimer(QWidget *parent) : QWidget(parent) {
  //QChronoTimer, if we want to go deeper in precision
  timer = new QTimer(this);
  timer->setTimerType(Qt::PreciseTimer);
  connect(timer, &QTimer::timeout, this, &PreciseTimer::updateTimer);
  fps = 30;
  timer->setInterval(this->calculateInterval());
  setWindowTitle(tr("Precise Timer"));

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  /*mainLayout->setGeometry(QRect(QPoint(960,540),QPoint(1920,1080)));*/
  QFont mediumFont = QFont("Times",15);
  QHBoxLayout *controlsLayout = new QHBoxLayout(this);
	controlsLayout->setSizeConstraint(QLayout::SetNoConstraint);
  /*controlsLayout->setGeometry(QRect(0, 0, 960, 500));*/
  frameSelectTextLabel = new QLabel(this);
  frameSelectTextLabel->setText("Framerate:");
  frameSelectTextLabel->setFont(mediumFont);

  startTimerBtn = new QPushButton(this);
  startTimerBtn->setText("Start Timer");
  connect(startTimerBtn, &QPushButton::pressed, this,
          &PreciseTimer::startFrameTimer);

  stopTimerBtn = new QPushButton(this);
  stopTimerBtn->setText("Stop Timer");
  connect(stopTimerBtn, &QPushButton::pressed, this,
          &PreciseTimer::stopFrameTimer);

  resetTimerBtn = new QPushButton(this);
  resetTimerBtn->setText("Reset Timer");
  connect(resetTimerBtn, &QPushButton::pressed, this,
          &PreciseTimer::resetFrameTimer);

  toggleShowFramesBtn = new QPushButton("Toggle Frame Label", this);
  connect(toggleShowFramesBtn, &QPushButton::pressed, this,
          &PreciseTimer::toggleFrameInfo);
  frameSelectSpinBox = new QSpinBox();
  frameSelectSpinBox->setRange(30, 240);
  frameSelectSpinBox->setSingleStep(5);
  frameSelectSpinBox->setValue(30);
  connect(frameSelectSpinBox, &QSpinBox::valueChanged, this,
          &PreciseTimer::changeFramerate);
  controlsLayout->addWidget(frameSelectTextLabel);
  controlsLayout->addWidget(frameSelectSpinBox);
  controlsLayout->addWidget(startTimerBtn);
  controlsLayout->addWidget(stopTimerBtn);
  controlsLayout->addWidget(resetTimerBtn);
	controlsLayout->addWidget(toggleShowFramesBtn);

  QHBoxLayout *labelsLayout = new QHBoxLayout(this);
  currentFrame = 0;

  frameTextLabel = new QLabel(this);
  frameTextLabel->setText("Frame: ");
  frameTextLabel->setFont(mediumFont);

  currentTimeTextLabel = new QLabel(this);
  currentTimeTextLabel->setText("Current Time: (Milliseconds Since Epoch)");
  currentTimeTextLabel->setFont(mediumFont);
  labelsLayout->addWidget(frameTextLabel);
  labelsLayout->addWidget(currentTimeTextLabel);


  QFont largeFont = QFont("Times",30,QFont::Bold);
  QHBoxLayout *countersLayout = new QHBoxLayout(this);
  frameIndicatorLabel = new QLabel(this);
  frameIndicatorLabel->setText(QString::number(0));
  frameIndicatorLabel->setFont(largeFont);

  currentTimeLabel = new QLabel(this);
  currentTimeLabel->setText(QString::number(QDateTime::currentMSecsSinceEpoch()));
  currentTimeLabel->setFont(largeFont);
  currentTimeLabel->setAlignment(Qt::AlignCenter);
  countersLayout->addWidget(frameIndicatorLabel);
  countersLayout->addWidget(currentTimeLabel);

  mainLayout->addLayout(controlsLayout);
  mainLayout->addLayout(labelsLayout);
  mainLayout->addLayout(countersLayout);
  setLayout(mainLayout);
  //half the screen width and all of the height
  //QGuiApplication.primaryScreen().size();
  this->move(960,0);
  resize(960, 1080);
  /*this->move(QPoint(400,400));*/
}

qint8 PreciseTimer::calculateInterval() { return 1000 / fps; }

void PreciseTimer::toggleFrameInfo() {
  if (frameIndicatorLabel->isVisible() && frameTextLabel->isVisible()) {
    frameIndicatorLabel->hide();
    frameTextLabel->hide();
  } else {
    frameIndicatorLabel->show();
    frameTextLabel->show();
  }
}

void PreciseTimer::changeFramerate(int newFrameRate) {
  stopFrameTimer();
  fps = newFrameRate;
}

void PreciseTimer::updateTimer() {

  // for displaying [1,fps]
  currentFrame = ((currentFrame) % fps) + 1;
  frameIndicatorLabel->setText(QString::number(currentFrame));
  currentTimeLabel->setText(QString::number(QDateTime::currentMSecsSinceEpoch()));
}

void PreciseTimer::stopFrameTimer() {
  if (timer->isActive()) {
    timer->stop();
  }
}

void PreciseTimer::startFrameTimer() {
  if (!timer->isActive()) {
    timer->start(this->calculateInterval());
  }
}

void PreciseTimer::resetFrameTimer() {

  timer->start(this->calculateInterval());
}

void PreciseTimer::paintEvent(QPaintEvent *event) {}
