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

#include <QWidget>

#include "core/UBSettings.h"

class QButtonGroup;
class QCheckBox;
class QGridLayout;
class QFontComboBox;
class QLabel;
class QToolButton;
class QVBoxLayout;
class UBColorSwatches;
class UBPictureChoice;
class UBSegmentedChoice;
class UBShapeChoice;
class UBStrokePreview;
class UBValueSlider;

/**
 * @brief Base of the panels where the user sets the options of one tool,
 * laid out like the tool panels of Drawboard PDF.
 *
 * A panel reads its values from the settings when it is shown and writes
 * every change back immediately, so that the next stroke uses it.
 */
class UBToolOptionsPanel : public QWidget
{
    Q_OBJECT

public:
    /** @brief Returns a new panel for the tool, or nullptr when the tool has no options. */
    static UBToolOptionsPanel* create(int tool, QWidget* parent = nullptr);

    /** @brief Shows the current values of the settings. */
    virtual void refresh() = 0;

    /** @brief Lays the options out in one row, for the toolbar. */
    void setHorizontal();

protected:
    UBToolOptionsPanel(const QString& title, QWidget* parent);

    /** @brief Called by setHorizontal() for the changes a panel needs to fit in the toolbar. */
    virtual void layOutForToolbar() {}

    void addRow(QWidget* widget);
    void addSection(const QString& title);

    QVBoxLayout* mLayout{nullptr};
    QLabel* mTitle{nullptr};
};

class UBPenOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBPenOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

protected:
    void layOutForToolbar() override;

private:
    void updatePreview();

    UBStrokePreview* mPreview{nullptr};
    UBSegmentedChoice* mStrokeType{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBColorSwatches* mColors{nullptr};
    UBValueSlider* mOpacity{nullptr};
    QCheckBox* mRecognizeShapes{nullptr};
    QCheckBox* mRecognizeHandwriting{nullptr};
};

class UBEraserOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBEraserOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    UBSegmentedChoice* mMode{nullptr};
    UBValueSlider* mSize{nullptr};
    QToolButton* mEraseAll{nullptr};
};

class UBMarkerOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBMarkerOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    void updatePreview();

    UBStrokePreview* mPreview{nullptr};
    UBColorSwatches* mColors{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBValueSlider* mOpacity{nullptr};
    UBSegmentedChoice* mTip{nullptr};
};

class UBLaserOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBLaserOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

protected:
    void layOutForToolbar() override;

private:
    void updatePreview();

    UBStrokePreview* mPreview{nullptr};
    UBSegmentedChoice* mMode{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBColorSwatches* mColors{nullptr};
    UBSegmentedChoice* mEffect{nullptr};
    UBValueSlider* mOpacity{nullptr};
    UBValueSlider* mDelay{nullptr};
    UBValueSlider* mDuration{nullptr};
};

/** @brief Options of the rectangle, ellipse, arrow and underline tools. */
class UBShapeOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    UBShapeOptionsPanel(int tool, QWidget* parent = nullptr);
    void refresh() override;

protected:
    void layOutForToolbar() override;

private:
    void showColorTarget(int target);

    UBSettings::ShapeSettings* mSettings{nullptr};
    int mTool{0};
    UBShapeChoice* mShapes{nullptr};
    UBSegmentedChoice* mColorTarget{nullptr};
    UBColorSwatches* mStrokeColors{nullptr};
    UBColorSwatches* mFillColors{nullptr};
    UBValueSlider* mWidth{nullptr};
    UBPictureChoice* mDash{nullptr};
    UBPictureChoice* mBorderStyle{nullptr};
    UBPictureChoice* mStartEnding{nullptr};
    UBPictureChoice* mEndEnding{nullptr};
    QCheckBox* mTapered{nullptr};
    QCheckBox* mFitText{nullptr};
    UBValueSlider* mOpacity{nullptr};
};

/** @brief The color of the next notes. */
class UBNoteOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBNoteOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    UBColorSwatches* mColors{nullptr};
};

/**
 * @brief Options of the emoji tool: the emoji to put and its size, or the
 * GIFs of klipy.com in a window beside the board, to drag to the page.
 */
class UBEmojiOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBEmojiOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

protected:
    void layOutForToolbar() override;

private:
    void layOutEmojis(int columns);
    void showGifs(bool shown);
    void onGifWindowClosed();

    UBSegmentedChoice* mMode{nullptr};
    QWidget* mEmojis{nullptr};
    QButtonGroup* mEmojiButtons{nullptr};
    QLabel* mSizeTitle{nullptr};
    UBValueSlider* mSize{nullptr};
    QLabel* mDragHint{nullptr};
};

/**
 * @brief Options of the AI tool: what to give to an AI on the web. On the
 * desktop, a part of the screen is given and there is no transcription.
 */
class UBAiOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBAiOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

protected:
    void layOutForToolbar() override;

private:
    void placeTiles(int columns);

    QGridLayout* mTiles{nullptr};
    QList<QToolButton*> mButtons;
    QToolButton* mTranscribe{nullptr};
    int mColumns{3};
};

/** @brief Options of the measurements: distance, perimeter and area; the one in use chosen again ends them. */
class UBMeasureOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBMeasureOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    QList<QToolButton*> mButtons;
};

/** @brief Options of the read aloud tool: where to read from, play, pause, stop and next page. */
class UBVoiceOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBVoiceOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    UBSegmentedChoice* mFrom{nullptr};
};

/** @brief Options of the attachment tool: a file, or a hyperlink to a web page. */
class UBAttachmentOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBAttachmentOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

    /** @brief Shows on the tool the kind it puts: a paper clip or a link. */
    static void updateToolIcon(bool desktop);

private:
    UBSegmentedChoice* mKind{nullptr};
};

/** @brief Options of the spotlight and the magnifier. */
class UBSpotlightOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBSpotlightOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

    /** @brief Shows on the tool a flashlight or a magnifying glass. */
    static void updateToolIcon();

private:
    void onModeClicked(int mode);
    void showWhetherShown();
    void showModeOptions();

    UBSegmentedChoice* mMode{nullptr};
    UBValueSlider* mRadius{nullptr};
    QLabel* mDarknessTitle{nullptr};
    UBValueSlider* mDarkness{nullptr};
    QLabel* mZoomTitle{nullptr};
    UBValueSlider* mZoom{nullptr};
};

class UBStampOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBStampOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

protected:
    void layOutForToolbar() override;

private:
    void choose(int index);
    void chooseCustomPicture();
    QIcon iconFor(int index) const;

    QToolButton* mRemove{nullptr};
    QList<QToolButton*> mStamps;
    QGridLayout* mGallery{nullptr};
};

/** @brief The selector: a rectangle or a lasso, the other one offered by a button. */
class UBSelectorOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBSelectorOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    UBSegmentedChoice* mMode{nullptr};
    QLabel* mHint{nullptr};
};

/** @brief The quick notes of the document: previous, next, list, show or hide, merge. */
class UBCommentOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBCommentOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    QToolButton* mShow{nullptr};
};

class UBTextOptionsPanel : public UBToolOptionsPanel
{
    Q_OBJECT

public:
    explicit UBTextOptionsPanel(QWidget* parent = nullptr);
    void refresh() override;

private:
    QFontComboBox* mFont{nullptr};
    UBValueSlider* mSize{nullptr};
    UBColorSwatches* mColors{nullptr};
    QToolButton* mBold{nullptr};
    QToolButton* mItalic{nullptr};
};
