
#include <QtWidgets>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
class PreciseTimer: public QWidget {
	Q_OBJECT

public: 
	explicit PreciseTimer(QWidget *parent = nullptr);

protected:
	void paintEvent(QPaintEvent *event) override;


private:

	QTimer *timer;
	qint8 fps;
	qint8 currentFrame;
	QLabel *frameTextLabel;
	QLabel *currentTimeTextLabel;
	QLabel *frameIndicatorLabel;
	QLabel *currentTimeLabel;
	QPushButton *startTimerBtn;
	QPushButton *stopTimerBtn;
	QPushButton *resetTimerBtn;
	QPushButton *toggleShowFramesBtn;
	QLabel *frameSelectTextLabel;
	QSpinBox *frameSelectSpinBox;

	void toggleFrameInfo();

	void changeFramerate(int newFramerate);
	qint8 calculateInterval();
	void startFrameTimer();
	void stopFrameTimer();
	void updateTimer();
	void resetFrameTimer();

};
