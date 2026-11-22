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

#include <QList>
#include <QObject>
#include <QPolygonF>
#include <QTimer>
#include <memory>

class UBGraphicsScene;
class UBGraphicsStrokesGroup;

/**
 * @brief Turns the handwriting of the pen into a text box: with Recognize
 * and convert handwriting ticked in the options of the pen, the words
 * written are read by the handwriting recognition of Windows a moment
 * after the pen rests, and replaced by the same text, of their size and
 * color, as a text box.
 */
class UBHandwriting : public QObject
{
    Q_OBJECT

public:
    static UBHandwriting* instance();

    /** @brief The recognition of handwriting is there: Windows 10 or 11. */
    static bool isAvailable();

    static bool isEnabled();
    static void setEnabled(bool enabled);

    /** @brief Lines just written with the pen, to read once the pen rests. */
    void addWritten(std::shared_ptr<UBGraphicsScene> scene, const QList<UBGraphicsStrokesGroup*>& groups);

private:
    UBHandwriting();

    void recognizePending();
    void replaceByText(std::weak_ptr<UBGraphicsScene> scene, QList<UBGraphicsStrokesGroup*> groups, const QString& text);

    /** @brief The text of the strokes, by the recognizer of the language given; empty when nothing is read. */
    static QString recognize(const QList<QPolygonF>& strokes, const QString& language);

    QTimer mRest;
    std::weak_ptr<UBGraphicsScene> mScene;
    QList<UBGraphicsStrokesGroup*> mPending;
};
