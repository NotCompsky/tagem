#include <QtWidgets>
#include <QDebug>

#include "movieplayer.h"

MoviePlayer::MoviePlayer(QWidget *parent)
    : QWidget(parent)
{
    movie = new QMovie(this);
    movie->setCacheMode(QMovie::CacheAll);

    movieLabel = new QLabel(tr("No movie loaded"));
    movieLabel->setAlignment(Qt::AlignCenter);
    movieLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    movieLabel->setBackgroundRole(QPalette::Dark);
    movieLabel->setAutoFillBackground(true);

    createControls();
    createButtons();

    connect(movie, SIGNAL(frameChanged(int)), this, SLOT(updateFrameSlider()));
    connect(movie, SIGNAL(stateChanged(QMovie::MovieState)),
            this, SLOT(updateButtons()));
    connect(fitCheckBox, SIGNAL(clicked()), this, SLOT(fitToWindow()));
    connect(frameSlider, SIGNAL(valueChanged(int)), this, SLOT(goToFrame(int)));
    connect(speedSpinBox, SIGNAL(valueChanged(int)),
            movie, SLOT(setSpeed(int)));

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(movieLabel);
    mainLayout->addLayout(controlsLayout);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    updateFrameSlider();
    updateButtons();

    setWindowTitle(tr("Movie Player"));
    resize(1920/2, 1080);
    
    inf = fopen("/tmp/mpv.sock", "r");
}

void MoviePlayer::openFile(const QString &fileName){
    qDebug() << "openFile " << fileName;
    
    movie->stop();
    movieLabel->setMovie(movie);
    movie->setFileName(fileName);
    movie->start();

    updateFrameSlider();
    updateButtons();
}

void MoviePlayer::load_next_file(){
    char* fp;
    size_t fp_size;
    if (getline(&fp, &fp_size, inf) == -1)
        close();
    fp[strlen(fp)-1] = 0; // Remove \n char
    QString s = "";
    for (auto i=0; i<strlen(fp); ++i)
        s += fp[i];
    openFile(s);
}

void MoviePlayer::goToFrame(int frame)
{
    movie->jumpToFrame(frame);
}

void MoviePlayer::fitToWindow()
{
    movieLabel->setScaledContents(fitCheckBox->isChecked());
}

void MoviePlayer::updateFrameSlider()
{
    bool hasFrames = (movie->currentFrameNumber() >= 0);

    if (hasFrames) {
        if (movie->frameCount() > 0) {
            frameSlider->setMaximum(movie->frameCount() - 1);
        } else {
            if (movie->currentFrameNumber() > frameSlider->maximum())
                frameSlider->setMaximum(movie->currentFrameNumber());
        }
        frameSlider->setValue(movie->currentFrameNumber());
    } else {
        frameSlider->setMaximum(0);
    }
    frameLabel->setEnabled(hasFrames);
    frameSlider->setEnabled(hasFrames);
}

void MoviePlayer::updateButtons()
{
    playButton->setEnabled(movie->isValid() && movie->frameCount() != 1
                           && movie->state() == QMovie::NotRunning);
    pauseButton->setEnabled(movie->state() != QMovie::NotRunning);
    pauseButton->setChecked(movie->state() == QMovie::Paused);
    nextButton->setEnabled(true);
}

void MoviePlayer::createControls()
{
    fitCheckBox = new QCheckBox(tr("Fit to Window"));

    frameLabel = new QLabel(tr("Current frame:"));

    frameSlider = new QSlider(Qt::Horizontal);
    frameSlider->setTickPosition(QSlider::TicksBelow);
    frameSlider->setTickInterval(10);

    speedLabel = new QLabel(tr("Speed:"));

    speedSpinBox = new QSpinBox;
    speedSpinBox->setRange(1, 9999);
    speedSpinBox->setValue(100);
    speedSpinBox->setSuffix(tr("%"));

    controlsLayout = new QGridLayout;
    controlsLayout->addWidget(fitCheckBox, 0, 0, 1, 2);
    controlsLayout->addWidget(frameLabel, 1, 0);
    controlsLayout->addWidget(frameSlider, 1, 1, 1, 2);
    controlsLayout->addWidget(speedLabel, 2, 0);
    controlsLayout->addWidget(speedSpinBox, 2, 1);
}

void MoviePlayer::createButtons()
{
    QSize iconSize(36, 36);

    playButton = new QToolButton;
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playButton->setIconSize(iconSize);
    playButton->setToolTip(tr("Play"));
    connect(playButton, SIGNAL(clicked()), movie, SLOT(start()));

    pauseButton = new QToolButton;
    pauseButton->setCheckable(true);
    pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    pauseButton->setIconSize(iconSize);
    pauseButton->setToolTip(tr("Pause"));
    connect(pauseButton, SIGNAL(clicked(bool)), movie, SLOT(setPaused(bool)));

    nextButton = new QToolButton;
    nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    nextButton->setIconSize(iconSize);
    nextButton->setToolTip(tr("Next"));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(load_next_file()));

    quitButton = new QToolButton;
    quitButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    quitButton->setIconSize(iconSize);
    quitButton->setToolTip(tr("Quit"));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(playButton);
    buttonsLayout->addWidget(pauseButton);
    buttonsLayout->addWidget(nextButton);
    buttonsLayout->addWidget(quitButton);
    buttonsLayout->addStretch();
}
