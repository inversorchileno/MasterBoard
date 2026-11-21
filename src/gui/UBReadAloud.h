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

#pragma once

#include <QObject>
#include <QPointF>
#include <QRectF>

class QTextToSpeech;

/**
 * @brief Reads the pages aloud with the voices of the system.
 *
 * It reads from the start of the page shown and goes on with the next
 * pages, or from the word clicked on, or reads the text inside a rectangle
 * dragged on the page, which is marked while it is read, or the texts
 * selected. It can pause, go on, stop and skip to the next page.
 */
class UBReadAloud : public QObject
{
    Q_OBJECT

public:
    /** @brief The one voice of Master Board. */
    static UBReadAloud* instance();

    bool isReading() const;
    bool isPaused() const;

    /** @brief Reads the texts selected, or else the page; stops when it is reading. For the menu. */
    void readOrStop();

    /** @brief Reads from the start of the page shown, then the next pages; goes on after a pause. */
    void play();

    void pause();
    void stop();

    /** @brief Goes to the next page and reads it. */
    void nextPage();

    /** @brief Reads the text inside a rectangle of the page shown, in the coordinates of the scene. */
    void readArea(const QRectF& sceneRect);

    /** @brief Reads from the word at this point of the page shown, then the next pages. */
    void readFrom(const QPointF& scenePoint);

    /** @brief The rectangle being read, in the coordinates of the scene; empty when none. */
    QRectF readingArea() const { return mReadingArea; }

signals:
    void readingChanged(bool reading);
    void readingAreaChanged();

private:
    explicit UBReadAloud(QObject* parent = nullptr);

    void say(const QString& text);
    void onStateChanged();
    void setReadingArea(const QRectF& sceneRect);
    static QString pageText(bool selectionFirst);
    static QString areaText(const QRectF& sceneRect);
    static QString textFrom(const QPointF& scenePoint);

    QTextToSpeech* mSpeech{nullptr};
    QString mPendingText;
    bool mContinuous{false};    // goes on with the next pages
    bool mStopping{false};
    bool mWasSpeaking{false};
    QRectF mReadingArea;
};
