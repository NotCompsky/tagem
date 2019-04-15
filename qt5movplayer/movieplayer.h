
#ifndef MOVIEPLAYER_H
#define MOVIEPLAYER_H

#include <QWidget>

class QCheckBox;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QMovie;
class QSlider;
class QSpinBox;
class QToolButton;
class QVBoxLayout;

class MoviePlayer : public QWidget
{
    Q_OBJECT

public:
    MoviePlayer(QWidget *parent = 0);
    void openFile(const QString &fileName);

private slots:
    void load_next_file();
    void goToFrame(int frame);
    void fitToWindow();
    void updateButtons();
    void updateFrameSlider();

private:
    void createControls();
    void createButtons();

    QLabel *movieLabel;
    QMovie *movie;
    QToolButton *playButton;
    QToolButton *pauseButton;
    QToolButton *nextButton;
    QToolButton *quitButton;
    QCheckBox *fitCheckBox;
    QSlider *frameSlider;
    QSpinBox *speedSpinBox;
    QLabel *frameLabel;
    QLabel *speedLabel;

    QGridLayout *controlsLayout;
    QHBoxLayout *buttonsLayout;
    QVBoxLayout *mainLayout;
    
    FILE* inf;
};

#endif
