/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBSoundDialog.h"

#include "gui/UBSoundWave.h"

#include <QAudioInput>
#include <QAudioOutput>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMediaCaptureSession>
#include <QMediaFormat>
#include <QMediaPlayer>
#include <QMediaRecorder>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QStandardPaths>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

UBSoundDialog::UBSoundDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Sound"));

    mPlayer = new QMediaPlayer(this);
    mOutput = new QAudioOutput(this);
    mPlayer->setAudioOutput(mOutput);

    mProgress = new QProgressBar(this);
    mProgress->setTextVisible(false);
    mProgress->setRange(0, 1000);
    mProgress->setFixedHeight(12);

    mStatus = new QLabel(tr("Record a sound or choose an audio file."), this);
    mWave = new UBSoundWave(this);

    auto makeButton = [this](const QString& symbol, const QString& tip, const QString& color) {
        auto button = new QToolButton(this);
        button->setText(symbol);
        button->setToolTip(tip);
        button->setStyleSheet(QString("QToolButton { font-size: 20px; color: %1; border: none; padding: 2px 8px; }"
                                      "QToolButton:disabled { color: #b5b5b5; }"
                                      "QToolButton:hover { background: #e4e9f2; border-radius: 3px; }").arg(color));
        return button;
    };

    mPlay = makeButton(QStringLiteral("\u25B6"), tr("Play"), "#2f7fe0");
    mStop = makeButton(QStringLiteral("\u25A0"), tr("Stop"), "#2f7fe0");
    mRecord = makeButton(QStringLiteral("\u25CF"), tr("Record with the microphone"), "#e53935");

    auto browse = new QPushButton(tr("Browse..."), this);

    auto volume = new QSlider(Qt::Horizontal, this);
    volume->setRange(0, 100);
    volume->setValue(100);
    volume->setToolTip(tr("Volume"));
    volume->setMinimumWidth(140);

    auto controls = new QHBoxLayout();
    controls->addWidget(mPlay);
    controls->addWidget(mStop);
    controls->addWidget(mRecord);
    controls->addSpacing(8);
    controls->addWidget(browse);
    controls->addStretch(1);
    auto speaker = new QLabel(this);
    speaker->setPixmap(QIcon(":/images/marks/sound.svg").pixmap(QSize(20, 20)));
    controls->addWidget(speaker);
    controls->addWidget(volume);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mAccept = buttons->button(QDialogButtonBox::Ok);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(mWave);
    layout->addWidget(mProgress);
    layout->addLayout(controls);
    layout->addWidget(mStatus);
    layout->addWidget(buttons);

    connect(mPlay, &QToolButton::clicked, this, &UBSoundDialog::play);
    connect(mStop, &QToolButton::clicked, this, &UBSoundDialog::stop);
    connect(mRecord, &QToolButton::clicked, this, &UBSoundDialog::record);
    connect(browse, &QPushButton::clicked, this, &UBSoundDialog::browse);
    connect(volume, &QSlider::valueChanged, this, [this](int value) { mOutput->setVolume(value / 100.0); });
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() { stop(); accept(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(mPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        const qint64 duration = mPlayer->duration();
        mProgress->setValue(duration > 0 ? int(position * 1000 / duration) : 0);
    });
    connect(mPlayer, &QMediaPlayer::playbackStateChanged, this, &UBSoundDialog::updateButtons);

    setMinimumWidth(460);
    updateButtons();
}

UBSoundDialog::~UBSoundDialog()
{
    stop();

    // A recording not kept is not left behind.
    if (!mRecordingFile.isEmpty() && (result() != QDialog::Accepted || mFile != mRecordingFile))
    {
        QFile::remove(mRecordingFile);
    }
}

QString UBSoundDialog::soundFile() const
{
    return mFile;
}

void UBSoundDialog::play()
{
    if (mFile.isEmpty())
    {
        return;
    }

    mPlayer->setSource(QUrl::fromLocalFile(mFile));
    mPlayer->play();
}

void UBSoundDialog::stop()
{
    mPlayer->stop();

    if (mRecorder && mRecorder->recorderState() != QMediaRecorder::StoppedState)
    {
        mRecorder->stop();
    }
}

void UBSoundDialog::record()
{
    stop();

    if (!mRecorder)
    {
        mSession = new QMediaCaptureSession(this);
        mInput = new QAudioInput(this);
        mRecorder = new QMediaRecorder(this);
        mSession->setAudioInput(mInput);
        mSession->setRecorder(mRecorder);

        QMediaFormat format(QMediaFormat::Mpeg4Audio);
        format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
        mRecorder->setMediaFormat(format);
        mRecorder->setQuality(QMediaRecorder::HighQuality);

        connect(mRecorder, &QMediaRecorder::recorderStateChanged, this, [this](QMediaRecorder::RecorderState state) {
            if (state == QMediaRecorder::RecordingState)
            {
                mWave->start();
            }
            else
            {
                mWave->stop();
            }

            if (state == QMediaRecorder::StoppedState && QFileInfo(mRecorder->actualLocation().toLocalFile()).size() > 0)
            {
                mRecordingFile = mRecorder->actualLocation().toLocalFile();
                setSoundFile(mRecordingFile);
            }

            updateButtons();
        });
        connect(mRecorder, &QMediaRecorder::durationChanged, this, [this](qint64 duration) {
            mStatus->setText(tr("Recording... %1 s").arg(duration / 1000));
        });
        connect(mRecorder, &QMediaRecorder::errorOccurred, this, [this](QMediaRecorder::Error, const QString& message) {
            mStatus->setText(tr("The microphone could not be used: %1").arg(message));
            updateButtons();
        });
    }

    const QString folder = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString file = folder + "/Master Board " + tr("Recording") + " "
                         + QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss") + ".m4a";
    mRecorder->setOutputLocation(QUrl::fromLocalFile(file));
    mRecorder->record();
    mStatus->setText(tr("Recording... press Stop to end."));
    updateButtons();
}

void UBSoundDialog::browse()
{
    stop();

    const QString file = QFileDialog::getOpenFileName(this, tr("Choose an audio file"),
        QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
        tr("Audio files (*.mp3 *.wav *.m4a *.aac *.ogg *.wma *.flac);;All files (*)"));

    if (!file.isEmpty())
    {
        setSoundFile(file);
    }
}

void UBSoundDialog::setSoundFile(const QString& file)
{
    mFile = file;
    mProgress->setValue(0);
    mStatus->setText(QFileInfo(file).fileName());
    updateButtons();
}

void UBSoundDialog::updateButtons()
{
    const bool recording = mRecorder && mRecorder->recorderState() == QMediaRecorder::RecordingState;
    const bool playing = mPlayer->playbackState() == QMediaPlayer::PlayingState;

    mPlay->setEnabled(!mFile.isEmpty() && !recording && !playing);
    mStop->setEnabled(recording || playing);
    mRecord->setEnabled(!recording && !playing);
    mAccept->setEnabled(!mFile.isEmpty() && !recording);
}
