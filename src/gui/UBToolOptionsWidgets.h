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

#include <QColor>
#include <QIcon>
#include <QList>
#include <QStringList>
#include <QWidget>

class QButtonGroup;
class QComboBox;
class QSlider;
class QToolButton;

/**
 * @brief A row of color swatches with a button to choose any other color.
 *
 * An optional "none" swatch lets the user choose no color at all, for
 * example to draw a shape without filling it.
 */
class UBColorSwatches : public QWidget
{
    Q_OBJECT

public:
    UBColorSwatches(const QList<QColor>& colors, bool offerNone, QWidget* parent = nullptr);

    /**
     * @brief The thirteen colors of the pen, also offered for the marker
     * and the notes: with the other color and the eyedropper, three full
     * rows of five in the toolbar.
     */
    static QList<QColor> standardColors();

    void setCurrentColor(const QColor& color);
    QColor currentColor() const;

    /** @brief Offers only the swatches, without the buttons for any other color and the eyedropper. */
    void hideOtherColorButton();

    /** @brief Lays the swatches out as small circles in three rows, to save width in the toolbar. */
    void setThreeRows();

signals:
    void colorChosen(const QColor& color);

private:
    void selectSwatch(const QColor& color);
    void chooseOtherColor();
    void takeColorOnScreen();

    QList<QColor> mColors;
    QList<QToolButton*> mSwatches;
    QToolButton* mOtherColorButton{nullptr};
    QToolButton* mEyedropperButton{nullptr};
    QColor mCurrentColor;
};

/**
 * @brief A slider with an editable value box, for widths and opacities.
 *
 * The marks of the slider are evenly spaced even when their values are
 * not, like the width slider of Drawboard PDF: small widths get as much
 * room as large ones.
 */
class UBValueSlider : public QWidget
{
    Q_OBJECT

public:
    UBValueSlider(const QList<qreal>& marks, const QString& unit, int decimals, QWidget* parent = nullptr);

    void setValue(qreal value);
    qreal value() const;

    /** @brief A shorter slider, for the toolbar. */
    void setCompact();

    /** @brief Gives the slider itself this width, its whole range seen. */
    void setSliderWidth(int width);

    /**
     * @brief Puts the caption, the value and the slider one under the other,
     * so that the slider takes little width in the toolbar.
     */
    void setStacked(const QString& caption);

signals:
    void valueChanged(qreal value);

private:
    int positionOf(qreal value) const;
    qreal valueAt(int position) const;
    void onSliderMoved(int position);
    void onTextEdited();
    void showValue(qreal value);

    QList<qreal> mMarks;
    QString mUnit;
    int mDecimals{0};
    qreal mValue{0};
    QSlider* mSlider{nullptr};
    QComboBox* mValueBox{nullptr};
};

/**
 * @brief A row of buttons where exactly one is chosen, like the "Básico /
 * Pro / Caligrafía" choice of Drawboard PDF.
 */
class UBSegmentedChoice : public QWidget
{
    Q_OBJECT

public:
    explicit UBSegmentedChoice(const QStringList& labels, QWidget* parent = nullptr);

    void setChoice(int index);
    int choice() const;

    /** @brief Leaves every button unchosen, for a choice that is also a switch. */
    void clearChoice();

    /** @brief Shows a drawing above the label of each button, like the tiles of Drawboard PDF. */
    void setIcons(const QList<QIcon>& icons, Qt::ToolButtonStyle style = Qt::ToolButtonTextUnderIcon);

    /** @brief Shows only the first line of each label, with the whole label as tooltip. */
    void setCompact();

    /** @brief Smaller still: small drawings and text, for many choices in the toolbar. */
    void setSmall();

    /** @brief One small button under the other, to take little width in the toolbar. */
    void setVertical();

signals:
    void choiceChanged(int index);

private:
    void equalizeWidths();

    QButtonGroup* mGroup{nullptr};
    bool mCompact{false};
    bool mSmall{false};
};

/** @brief The drawings shown on the choices of the tool options. */
enum class UBOptionIcon
{
    BasicStroke,
    ProStroke,
    CalligraphyStroke,
    Border,
    Fill,
    LaserTrail,
    LaserPoint,
    LaserFollow,
    Spotlight,
    Magnifier,
    LaserStandard,
    LaserWhiteBurn,
    EraserWhole,
    EraserPartial,
    MarkerRoundTip,
    MarkerChiselTip,
    ReadFromStart,
    ReadSelection,
    ReadFromClick,
    Play,
    Pause,
    Stop,
    NextPage,
    Microphone,
    SelectArrow,
    SelectLasso,
    Checklist,
    Bullets,
    Numbers,
    Table,
    AiSummary,
    AiTranslate,
    AiCreateImage,
    AiRemoveBackground,
    AiErase,
    AiTranscribe
};

QIcon optionIcon(UBOptionIcon kind);

/** @brief A picture of the tools as a drawing of the choices, as tall as the others. */
QIcon optionIcon(const QString& picture);

/** @brief The drawing of a pen of this UBPenStrokeType, its colored parts in this color. */
QIcon penStyleIcon(int strokeType, const QColor& color);

/**
 * @brief A choice among small drawings, for line dashes and line endings.
 */
class UBPictureChoice : public QWidget
{
    Q_OBJECT

public:
    enum class Kind
    {
        Dash,
        BorderStyle,
        StartEnding,
        EndEnding
    };

    UBPictureChoice(Kind kind, int count, QWidget* parent = nullptr);

    void setChoice(int index);
    int choice() const;

    /** @brief Shows only the chosen picture; a click on it drops down the others. */
    void setDropDown();

signals:
    void choiceChanged(int index);

private:
    QButtonGroup* mGroup{nullptr};
    QToolButton* mDropDown{nullptr};
};

/**
 * @brief The shapes of the shapes tool, as small drawings in rows: line,
 * polyline, ellipse, rectangle, triangles, arrows, stars, speech bubbles…
 */
class UBShapeChoice : public QWidget
{
    Q_OBJECT

public:
    explicit UBShapeChoice(QWidget* parent = nullptr);

    /** @brief The shape chosen, from UBShapeKind. */
    void setChoice(int kind);

    /** @brief Lays the small drawings out in this many columns, for the toolbar. */
    void setColumns(int columns);

    /** @brief Larger drawings in this many columns, taking all the width, for the options window. */
    void setWide(int columns);

signals:
    void choiceChanged(int kind);

private:
    QButtonGroup* mGroup{nullptr};
};

/**
 * @brief Shows what a stroke will look like with the current options.
 */
class UBStrokePreview : public QWidget
{
    Q_OBJECT

public:
    explicit UBStrokePreview(QWidget* parent = nullptr);

    void setStroke(const QColor& color, qreal width, qreal opacity, bool glow);

    /** @brief Draws the stroke as a basic, pro or calligraphy pen does, see UBPenStrokeType. */
    void setStrokeType(int type);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor mColor{Qt::red};
    qreal mWidth{3};
    qreal mOpacity{1};
    bool mGlow{false};
    int mStrokeType{0};
};

/** @brief The style sheet shared by the tool option panels. */
QString toolOptionsStyleSheet();
