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

#include "UBToolOptionsPanel.h"

#include <QButtonGroup>
#include <tuple>
#include <QCheckBox>
#include <QFileDialog>
#include <QFontComboBox>
#include <QGridLayout>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBSetting.h"
#include "core/UBSettings.h"
#include "board/UBComments.h"
#include "domain/UBGraphicsCommentItem.h"
#include "domain/UBEmojis.h"
#include "domain/UBGraphicsNoteItem.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBStamps.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolOptionsArrows.h"
#include "gui/UBToolOptionsWidgets.h"
#include "frameworks/UBPlatformUtils.h"
#include "board/UBBoardController.h"
#include "gui/UBDictation.h"
#include "gui/UBGifWindow.h"
#include "gui/UBAiPanel.h"
#include "core/UBApplicationController.h"
#include "gui/UBToolOptionsPopup.h"
#include "gui/UBTextBoxes.h"
#include "gui/UBReadAloud.h"
#include <QTimer>
#include "gui/UBTheme.h"
#include "gui/UBMeasureTool.h"

// Width of the eraser on screen for each point of its size
static constexpr qreal sEraserPixelsPerPoint = 3.0;

static QList<QColor> penColors()
{
    return UBColorSwatches::standardColors();
}

// The same colors as the pen, as Luis asked.
static QList<QColor> markerColors()
{
    return penColors();
}

static QList<QColor> laserColors()
{
    return { QColor("#ff2828"), QColor("#ff9a1f"), QColor("#ff1fd2"), QColor("#2bff2b"), QColor("#2f54ff"),
             QColor("#9a46f0"), QColor("#000000"), QColor("#14b85a"), QColor("#ffee1f") };
}

static QIcon rainbowIcon()
{
    // A wave in the colors of the rainbow, like the drawings of the other effects.
    const qreal ratio = 2;
    QPixmap pixmap(QSize(48, 28) * ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QLinearGradient gradient(5, 0, 43, 0);

    for (int i = 0; i <= 6; ++i)
    {
        gradient.setColorAt(i / 6.0, QColor::fromHslF(i / 7.0, 0.95, 0.6));
    }

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QBrush(gradient), 4, Qt::SolidLine, Qt::RoundCap));
    QPainterPath wave;
    wave.moveTo(5, 14);
    wave.cubicTo(16.4, -2.8, 31.6, 30.8, 43, 14);
    painter.drawPath(wave);

    return QIcon(pixmap);
}

static QColor settingColor(UBSetting* setting)
{
    return QColor(setting->get().toString());
}

static QString colorName(const QColor& color)
{
    return color.isValid() ? color.name() : QString();
}

static QList<qreal> opacityMarks()
{
    return { 0, 25, 50, 75, 100 };
}

UBToolOptionsPanel* UBToolOptionsPanel::create(int tool, QWidget* parent)
{
    switch (tool)
    {
        case UBStylusTool::Pen:
        case UBStylusTool::Line:
            return new UBPenOptionsPanel(parent);
        case UBStylusTool::Eraser:
            return new UBEraserOptionsPanel(parent);
        case UBStylusTool::Marker:
            return new UBMarkerOptionsPanel(parent);
        case UBStylusTool::Pointer:
            return new UBLaserOptionsPanel(parent);
        case UBStylusTool::Rectangle:
        case UBStylusTool::Ellipse:
        case UBStylusTool::Arrow:
        case UBStylusTool::Underline:
        case UBStylusTool::Highlight:
            return new UBShapeOptionsPanel(tool, parent);
        case UBStylusTool::Text:
            return new UBTextOptionsPanel(parent);
        case UBStylusTool::Note:
            return new UBNoteOptionsPanel(parent);
        case UBStylusTool::Stamp:
            return new UBStampOptionsPanel(parent);
        case UBStylusTool::Spotlight:
            return new UBSpotlightOptionsPanel(parent);
        case UBStylusTool::Attachment:
            return new UBAttachmentOptionsPanel(parent);
        case UBStylusTool::Voice:
            return new UBVoiceOptionsPanel(parent);
        case UBStylusTool::Emoji:
            return new UBEmojiOptionsPanel(parent);
        case UBStylusTool::Ai:
            return new UBAiOptionsPanel(parent);
        case UBStylusTool::Measure:
            return new UBMeasureOptionsPanel(parent);
        case UBStylusTool::Comment:
            return new UBCommentOptionsPanel(parent);
        case UBStylusTool::Selector:
            return new UBSelectorOptionsPanel(parent);
        default:
            return nullptr;
    }
}

UBToolOptionsPanel::UBToolOptionsPanel(const QString& title, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet(toolOptionsStyleSheet());
    setMinimumWidth(320);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(14, 12, 14, 14);
    mLayout->setSpacing(8);

    mTitle = new QLabel(title, this);
    mTitle->setObjectName("title");
    mLayout->addWidget(mTitle);
}

void UBToolOptionsPanel::setHorizontal()
{
    mLayout->setDirection(QBoxLayout::LeftToRight);
    mLayout->setContentsMargins(8, 2, 8, 2);
    mLayout->setSpacing(10);
    setMinimumWidth(0);
    setStyleSheet(toolOptionsStyleSheet() + "UBToolOptionsPanel { background: transparent; }");

    // The toolbar shows which tool is selected: no title and no preview.
    mTitle->hide();

    for (UBStrokePreview* preview : findChildren<UBStrokePreview*>())
    {
        preview->hide();
    }

    for (UBSegmentedChoice* choice : findChildren<UBSegmentedChoice*>())
    {
        choice->setCompact();
    }

    for (UBValueSlider* slider : findChildren<UBValueSlider*>())
    {
        slider->setCompact();
    }

    for (UBColorSwatches* swatches : findChildren<UBColorSwatches*>())
    {
        swatches->setThreeRows();
    }

    // The swatches say what they are for: no "Color" title before them.
    for (QLabel* label : findChildren<QLabel*>())
    {
        if (label->objectName() == "section" && label->text() == tr("Color"))
        {
            label->hide();
        }
    }

    if (stacksSlidersInToolbar())
    {
        // A section title just before a slider becomes its caption.
        QLabel* section = nullptr;

        for (int i = 0; i < mLayout->count(); ++i)
        {
            QWidget* widget = mLayout->itemAt(i)->widget();

            if (auto slider = qobject_cast<UBValueSlider*>(widget))
            {
                slider->setStacked(section ? section->text() : QString(), stacksValuesAboveSliders());

                if (section)
                {
                    section->hide();
                }
            }

            auto label = qobject_cast<QLabel*>(widget);
            section = (label && label->objectName() == "section") ? label : nullptr;
        }
    }

    layOutForToolbar();
    mLayout->addStretch();
}

void UBToolOptionsPanel::addRow(QWidget* widget)
{
    mLayout->addWidget(widget);
}

void UBToolOptionsPanel::addSection(const QString& title)
{
    auto label = new QLabel(title, this);
    label->setObjectName("section");
    mLayout->addSpacing(4);
    mLayout->addWidget(label);
}

// ---------------------------------------------------------------------------
// Pen

// Read by the scene when a stroke of the pen ends.
static const QString sRecognizeShapesKey = QStringLiteral("Board/LibertyBoardRecognizeShapes");

UBPenOptionsPanel::UBPenOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Pen"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mPreview = new UBStrokePreview(this);
    addRow(mPreview);

    // The pens of Samsung Notes, in the order of UBPenStrokeType.
    mStrokeType = new UBSegmentedChoice({ tr("Ballpoint pen"), tr("Fountain pen"), tr("Calligraphy pen"),
                                          tr("Pencil"), tr("Marker") }, this);
    mStrokeType->setToolTip(tr("Ballpoint pen: thicker when slow, thinner when fast. Fountain pen: a slanted flat nib. "
                               "Calligraphy pen: smooth and even. Pencil: thin and dark, with grain. Marker: wide, with a felt texture."));
    addRow(mStrokeType);

    mWidth = new UBValueSlider({ 1, 2, 3, 6, 10, 50 }, tr(" pt"), 1, this);
    addRow(mWidth);

    addSection(tr("Color"));
    mColors = new UBColorSwatches(penColors(), false, this);
    addRow(mColors);

    mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
    addRow(mOpacity);

    mRecognizeShapes = new QCheckBox(tr("Recognize shapes"), this);
    mRecognizeShapes->setToolTip(tr("A line, arrow, circle, oval, triangle, square or rectangle drawn by hand becomes a clean one."));
    addRow(mRecognizeShapes);

    connect(mRecognizeShapes, &QCheckBox::toggled, this, [settings](bool on) {
        settings->setValue(sRecognizeShapesKey, on);
    });

    connect(mStrokeType, &UBSegmentedChoice::choiceChanged, this, [this, settings](int type) {
        settings->penStrokeType->set(type);
        updatePreview();
    });
    connect(mWidth, &UBValueSlider::valueChanged, this, [this, settings](qreal width) {
        settings->setCurrentPenWidth(width);
        updatePreview();
    });
    connect(mColors, &UBColorSwatches::colorChosen, this, [this, settings](const QColor& color) {
        const int index = settings->penColorIndex();
        UBDrawingController::drawingController()->setPenColor(false, color, index);
        UBDrawingController::drawingController()->setPenColor(true, color, index);
        updatePreview();
    });
    connect(mOpacity, &UBValueSlider::valueChanged, this, [this, settings](qreal opacity) {
        settings->penOpacity->set(opacity / 100);
        updatePreview();
    });

    refresh();
}

void UBPenOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mStrokeType->setChoice(settings->penStrokeType->get().toInt());
    mWidth->setValue(settings->currentPenWidth());
    mColors->setCurrentColor(settings->currentPenColor());
    mOpacity->setValue(settings->penOpacity->get().toDouble() * 100);
    mRecognizeShapes->setChecked(settings->value(sRecognizeShapesKey, false).toBool());
    updatePreview();
}

void UBPenOptionsPanel::layOutForToolbar()
{
    // Five pens: smaller tiles, so that they leave room in the toolbar.
    mStrokeType->setSmall();
}

void UBPenOptionsPanel::updatePreview()
{
    // The pens take the color they draw with.
    QList<QIcon> icons;

    for (int type = UBPenStrokeType::Basic; type <= UBPenStrokeType::Marker; ++type)
    {
        icons << penStyleIcon(type, mColors->currentColor());
    }

    mStrokeType->setIcons(icons);

    mPreview->setStrokeType(mStrokeType->choice());
    mPreview->setStroke(mColors->currentColor(), mWidth->value(), mOpacity->value() / 100, false);
}

// ---------------------------------------------------------------------------
// Eraser

UBEraserOptionsPanel::UBEraserOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Eraser"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mMode = new UBSegmentedChoice({ tr("Whole"), tr("Partial") }, this);
    mMode->setIcons({ optionIcon(UBOptionIcon::EraserWhole), optionIcon(UBOptionIcon::EraserPartial) });
    mMode->setToolTip(tr("Whole: erases every stroke the eraser touches. Partial: erases only where the eraser passes."));
    addRow(mMode);

    mSize = new UBValueSlider({ 1, 2, 3, 6, 10, 50 }, tr(" pt"), 0, this);
    addRow(mSize);

    mEraseAll = new QToolButton(this);
    mEraseAll->setText(tr("Erase all annotations"));
    mEraseAll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addRow(mEraseAll);

    // Whole is shown first, like in Drawboard PDF, but is stored as its own value.
    connect(mMode, &UBSegmentedChoice::choiceChanged, this, [settings](int choice) {
        settings->eraserMode->set(choice == 0 ? int(UBEraserMode::Whole) : int(UBEraserMode::Partial));
    });
    connect(mSize, &UBValueSlider::valueChanged, this, [settings](qreal size) {
        settings->setCurrentEraserWidth(size * sEraserPixelsPerPoint);
    });
    connect(mEraseAll, &QToolButton::clicked, this, []() {
        if (UBDrawingController::drawingController()->isInDesktopMode())
        {
            UBApplication::mainWindow->actionEraseDesktopAnnotations->trigger();
        }
        else
        {
            UBApplication::mainWindow->actionEraseAnnotations->trigger();
        }
    });

    refresh();
}

void UBEraserOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mMode->setChoice(settings->eraserMode->get().toInt() == UBEraserMode::Whole ? 0 : 1);
    mSize->setValue(settings->currentEraserWidth() / sEraserPixelsPerPoint);
}

// ---------------------------------------------------------------------------
// Marker

UBMarkerOptionsPanel::UBMarkerOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Marker"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mPreview = new UBStrokePreview(this);
    addRow(mPreview);

    mColors = new UBColorSwatches(markerColors(), false, this);
    addRow(mColors);

    mWidth = new UBValueSlider({ 20, 30, 40, 50, 80, 100 }, tr(" pt"), 0, this);
    addRow(mWidth);

    mTip = new UBSegmentedChoice({ tr("Round tip"), tr("Straight tip") }, this);
    mTip->setIcons({ optionIcon(UBOptionIcon::MarkerRoundTip), optionIcon(UBOptionIcon::MarkerChiselTip) });
    mTip->setToolTip(tr("Round: the stroke has round ends. Straight: like a highlighter, wide across the lines of text, with straight ends."));
    addRow(mTip);

    addSection(tr("Advanced"));
    mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
    addRow(mOpacity);

    connect(mColors, &UBColorSwatches::colorChosen, this, [this, settings](QColor color) {
        color.setAlphaF(settings->boardMarkerAlpha->get().toDouble());
        const int index = settings->markerColorIndex();
        UBDrawingController::drawingController()->setMarkerColor(false, color, index);
        UBDrawingController::drawingController()->setMarkerColor(true, color, index);
        updatePreview();
    });
    connect(mWidth, &UBValueSlider::valueChanged, this, [this, settings](qreal width) {
        settings->setCurrentMarkerWidth(width);
        updatePreview();
    });
    connect(mOpacity, &UBValueSlider::valueChanged, this, [this](qreal opacity) {
        UBDrawingController::drawingController()->setMarkerAlpha(opacity / 100);
        updatePreview();
    });
    connect(mTip, &UBSegmentedChoice::choiceChanged, this, [settings](int tip) {
        settings->markerChiselTip->set(tip == 1);
    });

    refresh();
}

void UBMarkerOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mColors->setCurrentColor(settings->currentMarkerColor());
    mWidth->setValue(settings->currentMarkerWidth());
    mOpacity->setValue(settings->boardMarkerAlpha->get().toDouble() * 100);
    mTip->setChoice(settings->markerChiselTip->get().toBool() ? 1 : 0);
    updatePreview();
}

void UBMarkerOptionsPanel::updatePreview()
{
    // Shown thinner than drawn so that the preview stays readable.
    mPreview->setStroke(mColors->currentColor(), mWidth->value() / 4, mOpacity->value() / 100, false);
}

// ---------------------------------------------------------------------------
// Laser pointer

UBLaserOptionsPanel::UBLaserOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Laser pointer"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mPreview = new UBStrokePreview(this);
    addRow(mPreview);

    mMode = new UBSegmentedChoice({ tr("Trail"), tr("Point"), tr("Follow") }, this);
    mMode->setIcons({ optionIcon(UBOptionIcon::LaserTrail), optionIcon(UBOptionIcon::LaserPoint),
                      optionIcon(UBOptionIcon::LaserFollow) });
    mMode->setToolTip(tr("Trail: draws while pressed. Point: shows only the point. "
                         "Follow: follows the mouse without a click, its trail fading with the times below."));
    addRow(mMode);

    mWidth = new UBValueSlider({ 0.5, 1, 2, 4, 8, 16, 64 }, tr(" pt"), 1, this);
    addRow(mWidth);

    addSection(tr("Color"));
    mColors = new UBColorSwatches(laserColors(), false, this);
    addRow(mColors);

    // The rainbow is an effect beside the others, apart from the colors.
    mEffect = new UBSegmentedChoice({ tr("Standard"), tr("White burn"), tr("Rainbow") }, this);
    mEffect->setIcons({ optionIcon(UBOptionIcon::LaserStandard), optionIcon(UBOptionIcon::LaserWhiteBurn), rainbowIcon() });
    mEffect->setToolTip(tr("Rainbow: the trail takes the colors of the rainbow, one after the other."));
    addRow(mEffect);

    addSection(tr("Opacity"));
    mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
    addRow(mOpacity);

    addSection(tr("Wait before fading"));
    mDelay = new UBValueSlider({ 0, 0.1, 0.5, 1, 2, 5 }, tr(" s"), 1, this);
    addRow(mDelay);

    addSection(tr("Fading duration"));
    mDuration = new UBValueSlider({ 0.1, 0.5, 1, 2, 3, 5, 10 }, tr(" s"), 1, this);
    addRow(mDuration);

    connect(mMode, &UBSegmentedChoice::choiceChanged, this, [settings](int mode) {
        settings->laserMode->set(mode);
    });
    connect(mWidth, &UBValueSlider::valueChanged, this, [this, settings](qreal width) {
        settings->laserWidth->set(width);
        updatePreview();
    });
    connect(mColors, &UBColorSwatches::colorChosen, this, [this, settings](const QColor& color) {
        settings->laserColor->set(color.name());

        // A color is for the standard trail, not for the rainbow.
        if (settings->laserEffect->get().toInt() == UBLaserEffect::Rainbow)
        {
            settings->laserEffect->set(int(UBLaserEffect::Standard));
            mEffect->setChoice(UBLaserEffect::Standard);
        }

        updatePreview();
    });
    connect(mEffect, &UBSegmentedChoice::choiceChanged, this, [settings](int effect) {
        settings->laserEffect->set(effect);
    });
    connect(mOpacity, &UBValueSlider::valueChanged, this, [this, settings](qreal opacity) {
        settings->laserOpacity->set(opacity / 100);
        updatePreview();
    });
    connect(mDelay, &UBValueSlider::valueChanged, this, [settings](qreal delay) {
        settings->laserFadeDelay->set(delay);
    });
    connect(mDuration, &UBValueSlider::valueChanged, this, [settings](qreal duration) {
        settings->laserFadeDuration->set(duration);
    });

    refresh();
}

void UBLaserOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    mMode->setChoice(settings->laserMode->get().toInt());
    mWidth->setValue(settings->laserWidth->get().toDouble());
    mColors->setCurrentColor(settingColor(settings->laserColor));
    // The rainbow was a box of its own in build 20: it becomes the effect.
    if (settings->laserRainbow->get().toBool())
    {
        settings->laserEffect->set(int(UBLaserEffect::Rainbow));
        settings->laserRainbow->set(false);
    }

    mEffect->setChoice(settings->laserEffect->get().toInt());
    mOpacity->setValue(settings->laserOpacity->get().toDouble() * 100);
    mDelay->setValue(settings->laserFadeDelay->get().toDouble());
    mDuration->setValue(settings->laserFadeDuration->get().toDouble());
    updatePreview();
}

void UBLaserOptionsPanel::layOutForToolbar()
{
    // The two times take half the width they would take, with short
    // captions; the whole caption is the tooltip.
    const QList<QPair<UBValueSlider*, QString>> times = { { mDelay, tr("Wait") }, { mDuration, tr("Fading") } };

    for (const auto& time : times)
    {
        if (auto caption = time.first->findChild<QLabel*>("section"))
        {
            time.first->setToolTip(caption->text());
            caption->setText(time.second);
        }
    }

    // The sliders are all as wide, short enough for their handle to be seen.
    for (UBValueSlider* slider : { mWidth, mOpacity, mDelay, mDuration })
    {
        slider->setSliderWidth(110);
    }
}

void UBLaserOptionsPanel::updatePreview()
{
    mPreview->setStroke(mColors->currentColor(), mWidth->value(), mOpacity->value() / 100, true);
}

// ---------------------------------------------------------------------------
// Voice: reading aloud and dictation

UBEmojiOptionsPanel::UBEmojiOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Emojis and GIFs"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mMode = new UBSegmentedChoice({ tr("Emojis"), tr("GIFs") }, this);
    mMode->setToolTip(tr("GIFs: opens the page of klipy.com beside the board, without an account; "
                         "drag a GIF from it to the page. A second click on GIFs closes it."));
    addRow(mMode);

    // The emojis: a click chooses one, a click on the page puts it.
    mEmojis = new QWidget(this);
    mEmojiButtons = new QButtonGroup(mEmojis);
    QFont emojiFont;
    emojiFont.setFamilies({ "Segoe UI Emoji", "Apple Color Emoji", "Noto Color Emoji" });
    emojiFont.setPixelSize(16);

    for (const QString& emoji : UBEmojis::all())
    {
        auto button = new QToolButton(mEmojis);
        button->setText(emoji);
        button->setFont(emojiFont);
        button->setCheckable(true);
        button->setFixedSize(24, 22);
        button->setStyleSheet("QToolButton { background: transparent; border: 1px solid transparent; border-radius: 3px; "
                              "padding: 0px; margin: 0px; }"
                              "QToolButton:checked { background: rgba(255, 255, 255, 60); border-color: #e8e8e8; }");
        button->setStyleSheet(UBTheme::style(button->styleSheet()));
        mEmojiButtons->addButton(button);
    }

    layOutEmojis(8);
    addRow(mEmojis);

    // While the window of the GIFs is open.
    mDragHint = new QLabel(tr("Drag a GIF from the window of the GIFs to the page."), this);
    mDragHint->setObjectName("section");
    mDragHint->setStyleSheet("QLabel { color: white; font-size: 14px; }");
    addRow(mDragHint);

    addSection(tr("Size"));
    mSizeTitle = findChildren<QLabel*>("section").last();
    mSize = new UBValueSlider({ 32, 48, 64, 80, 120, 160, 240 }, tr(" pt"), 0, this);
    addRow(mSize);

    connect(mMode, &UBSegmentedChoice::choiceChanged, this, [this](int mode) {
        // GIFs opens the window, and closes it when it is open.
        if (mode == 1)
        {
            UBGifWindow::toggle(this);

            if (UBGifWindow::isOpen())
            {
                connect(UBGifWindow::instance(), &UBGifWindow::closed, this, &UBEmojiOptionsPanel::onGifWindowClosed,
                        Qt::UniqueConnection);
            }
        }

        showGifs(mode == 1 && UBGifWindow::isOpen());
    });
    connect(mEmojiButtons, &QButtonGroup::buttonClicked, this, [settings](QAbstractButton* button) {
        settings->emojiChoice->set(button->text());
    });
    connect(mSize, &UBValueSlider::valueChanged, this, [settings](qreal size) {
        settings->emojiSize->set(qRound(size));
    });

    refresh();
}

void UBEmojiOptionsPanel::layOutEmojis(int columns)
{
    delete mEmojis->layout();
    auto grid = new QGridLayout(mEmojis);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(1);
    const QList<QAbstractButton*> buttons = mEmojiButtons->buttons();

    for (int i = 0; i < buttons.size(); ++i)
    {
        grid->addWidget(buttons.at(i), i / columns, i % columns);
    }
}

void UBEmojiOptionsPanel::layOutForToolbar()
{
    // Three rows in the toolbar.
    layOutEmojis((UBEmojis::all().size() + 2) / 3);
    mEmojis->setFixedHeight(3 * 22 + 2);
}

void UBEmojiOptionsPanel::refresh()
{
    mSize->setValue(UBEmojis::size());

    for (QAbstractButton* button : mEmojiButtons->buttons())
    {
        button->setChecked(button->text() == UBEmojis::emoji());
    }

    showGifs(UBGifWindow::isOpen());
}

void UBEmojiOptionsPanel::onGifWindowClosed()
{
    showGifs(false);
}

void UBEmojiOptionsPanel::showGifs(bool shown)
{
    // The emojis, or the hint to drag the GIFs.
    mMode->setChoice(shown ? 1 : 0);
    mEmojis->setVisible(!shown);
    mSizeTitle->setVisible(!shown);
    mSize->setVisible(!shown);
    mDragHint->setVisible(shown);
}

UBAiOptionsPanel::UBAiOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("AI"), parent)
{
    // A tile for each action, its drawing above its name, all of the same
    // size: three by row in the options of the desktop, one row in the toolbar.
    const QList<std::tuple<int, QString, QIcon, QString>> actions = {
        { int(UBAiPanel::Action::Summary), tr("Summary"), optionIcon(UBOptionIcon::AiSummary),
          tr("A part of the page, or all of it, summarized by the AI") },
        { int(UBAiPanel::Action::Translation), tr("Translation"), optionIcon(UBOptionIcon::AiTranslate),
          tr("A part of the page, or all of it, translated by the AI into the language chosen") },
        { int(UBAiPanel::Action::CreateImage), tr("Create image"), optionIcon(UBOptionIcon::AiCreateImage),
          tr("An image made by the AI from what you write") },
        { int(UBAiPanel::Action::RemoveBackground), tr("Remove background"), optionIcon(UBOptionIcon::AiRemoveBackground),
          tr("A picture without its background, made by the AI") },
        { int(UBAiPanel::Action::GenerativeErase), tr("Generative erase"), optionIcon(UBOptionIcon::AiErase),
          tr("Something erased from a picture by the AI, as if it had never been there") },
        { int(UBAiPanel::Action::Transcription), tr("Transcribe"), optionIcon(UBOptionIcon::AiTranscribe),
          tr("Asks for an audio and gives it to the AI, which writes what is said") },
        { -1, tr("Open the AI"), optionIcon(":/images/stylusPalette/ai.svg"),
          tr("Opens or closes the page of the AI at the right") },
    };

    auto tiles = new QWidget(this);
    mTiles = new QGridLayout(tiles);
    mTiles->setContentsMargins(0, 0, 0, 0);
    mTiles->setSpacing(4);

    for (const auto& action : actions)
    {
        auto button = new QToolButton(tiles);
        button->setText(std::get<1>(action));
        button->setIcon(std::get<2>(action));
        button->setIconSize(QSize(48, 28));
        button->setToolTip(std::get<3>(action));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setFocusPolicy(Qt::NoFocus);
        mButtons << button;

        const int chosen = std::get<0>(action);

        if (chosen == int(UBAiPanel::Action::Transcription))
        {
            mTranscribe = button;
        }

        connect(button, &QToolButton::clicked, this, [this, chosen]() {
            // On the desktop the options are in a window of their own: it
            // goes, so that the screen can be chosen under it.
            QWidget* parent = window();

            if (qobject_cast<UBToolOptionsPopup*>(parent))
            {
                parent->hide();
                parent = parent->parentWidget() ? parent->parentWidget()->window() : nullptr;
            }

            if (chosen < 0)
            {
                UBAiPanel::toggle(parent);
            }
            else
            {
                UBAiPanel::startAction(UBAiPanel::Action(chosen), parent);
            }
        });
    }

    // All as wide as the longest name.
    int widest = 72;

    for (QToolButton* button : std::as_const(mButtons))
    {
        widest = qMax(widest, button->fontMetrics().horizontalAdvance(button->text()) + 18);
    }

    for (QToolButton* button : std::as_const(mButtons))
    {
        button->setFixedWidth(widest);
    }

    addRow(tiles);
    placeTiles(3);
    refresh();
}

void UBAiOptionsPanel::placeTiles(int columns)
{
    int index = 0;

    for (QToolButton* button : std::as_const(mButtons))
    {
        mTiles->removeWidget(button);

        if (!button->isHidden() || button != mTranscribe)
        {
            mTiles->addWidget(button, index / columns, index % columns);
            ++index;
        }
    }

    mColumns = columns;
}

void UBAiOptionsPanel::layOutForToolbar()
{
    placeTiles(mButtons.size());
}

void UBAiOptionsPanel::refresh()
{
    // No transcription on the desktop: there is no page to write it on.
    mTranscribe->setHidden(UBApplication::applicationController->isShowingDesktop());
    placeTiles(mColumns);
}

UBMeasureOptionsPanel::UBMeasureOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Measurements"), parent)
{
    const QList<std::tuple<UBMeasureTool::Mode, QString, QString, QString>> modes = {
        { UBMeasureTool::Mode::Distance, tr("Distance"), ":/images/measure/distance.svg",
          tr("The length of a line dragged from one end to the other") },
        { UBMeasureTool::Mode::Perimeter, tr("Perimeter"), ":/images/measure/perimeter.svg",
          tr("The length around a shape: a click at each corner, a double click at the last one") },
        { UBMeasureTool::Mode::Area, tr("Area"), ":/images/measure/area.svg",
          tr("The surface of a shape: a click at each corner, a double click at the last one") },
    };

    auto tiles = new QWidget(this);
    auto layout = new QHBoxLayout(tiles);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    for (const auto& mode : modes)
    {
        auto button = new QToolButton(tiles);
        button->setText(std::get<1>(mode));
        button->setIcon(QIcon(std::get<2>(mode)));
        button->setIconSize(QSize(28, 28));
        button->setToolTip(std::get<3>(mode));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setCheckable(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setFixedWidth(84);
        button->setProperty("mode", int(std::get<0>(mode)));
        layout->addWidget(button);
        mButtons << button;

        const UBMeasureTool::Mode chosen = std::get<0>(mode);

        connect(button, &QToolButton::clicked, this, [this, chosen]() {
            // On the desktop the options are in a window of their own: it goes.
            if (auto popup = qobject_cast<UBToolOptionsPopup*>(window()))
            {
                popup->hide();
            }

            UBMeasureTool::instance()->choose(chosen);
            refresh();
        });
    }

    connect(UBMeasureTool::instance(), &UBMeasureTool::modeChanged, this, &UBMeasureOptionsPanel::refresh);
    addRow(tiles);
    refresh();
}

void UBMeasureOptionsPanel::refresh()
{
    const int mode = int(UBMeasureTool::instance()->mode());

    for (QToolButton* button : std::as_const(mButtons))
    {
        button->setChecked(button->property("mode").toInt() == mode);
    }
}

UBVoiceOptionsPanel::UBVoiceOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Read aloud"), parent)
{
    UBSettings* settings = UBSettings::settings();
    UBReadAloud* voice = UBReadAloud::instance();

    // Every element is a tile with its drawing above its name. Reading:
    // from the start of the page, a rectangle dragged on the text, or the
    // word clicked.
    mFrom = new UBSegmentedChoice({ tr("From the start"), tr("Selected text"), tr("From the click") }, this);
    mFrom->setIcons({ optionIcon(UBOptionIcon::ReadFromStart), optionIcon(UBOptionIcon::ReadSelection),
                      optionIcon(UBOptionIcon::ReadFromClick) });
    mFrom->setToolTip(tr("From the start: reads the page shown, then the next pages. "
                         "Selected text: drag a rectangle over the text to read it. "
                         "From the click: click on a word to read from it, then the next pages."));
    addRow(mFrom);

    auto addButton = [this](const QString& text, UBOptionIcon icon, const QString& tip) {
        auto button = new QToolButton(this);
        button->setText(text);
        button->setIcon(optionIcon(icon));
        button->setIconSize(QSize(34, 19));
        button->setToolTip(tip);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setMinimumWidth(64);
        addRow(button);
        return button;
    };

    QToolButton* play = addButton(tr("Play"), UBOptionIcon::Play, tr("Reads the page from its start, or goes on after a pause"));
    QToolButton* pause = addButton(tr("Pause"), UBOptionIcon::Pause, tr("Pauses the reading"));
    QToolButton* stop = addButton(tr("Stop"), UBOptionIcon::Stop, tr("Stops the reading"));
    QToolButton* next = addButton(tr("Next page"), UBOptionIcon::NextPage, tr("Goes to the next page and reads it"));

    connect(mFrom, &UBSegmentedChoice::choiceChanged, this, [settings](int from) {
        settings->readFrom->set(from);
    });
    connect(play, &QToolButton::clicked, voice, &UBReadAloud::play);
    connect(pause, &QToolButton::clicked, voice, &UBReadAloud::pause);
    connect(stop, &QToolButton::clicked, voice, &UBReadAloud::stop);
    connect(next, &QToolButton::clicked, voice, &UBReadAloud::nextPage);

    refresh();
}

void UBVoiceOptionsPanel::refresh()
{
    mFrom->setChoice(qBound(0, UBSettings::settings()->readFrom->get().toInt(), 2));
}

// ---------------------------------------------------------------------------
// Attachment: a file or a hyperlink

UBAttachmentOptionsPanel::UBAttachmentOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Insert file"), parent)
{
    mKind = new UBSegmentedChoice({ tr("File"), tr("Hyperlink") }, this);
    mKind->setIcons({ optionIcon(":/images/stylusPalette/attachment.svg"), optionIcon(":/images/stylusPalette/hyperlink.svg") });
    mKind->setToolTip(tr("File: a click on the page attaches a file. Hyperlink: a click on the page asks for the address of a web page."));
    addRow(mKind);

    connect(mKind, &UBSegmentedChoice::choiceChanged, this, [](int kind) {
        UBSettings::settings()->attachmentHyperlink->set(kind == 1);
        updateToolIcon(UBDrawingController::drawingController()->isInDesktopMode());
    });

    refresh();
}

void UBAttachmentOptionsPanel::refresh()
{
    // On the desktop there is no document to keep a file: only hyperlinks.
    const bool desktop = UBDrawingController::drawingController()->isInDesktopMode();
    const bool hyperlink = desktop || UBSettings::settings()->attachmentHyperlink->get().toBool();
    mKind->setChoice(hyperlink ? 1 : 0);
    mKind->setEnabled(!desktop);
}

void UBAttachmentOptionsPanel::updateToolIcon(bool desktop)
{
    const bool hyperlink = desktop || UBSettings::settings()->attachmentHyperlink->get().toBool();
    QIcon icon;
    icon.addFile(hyperlink ? ":/images/stylusPalette/hyperlink.svg" : ":/images/stylusPalette/attachment.svg", QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(hyperlink ? ":/images/stylusPalette/hyperlinkOn.svg" : ":/images/stylusPalette/attachmentOn.svg", QSize(), QIcon::Normal, QIcon::On);
    UBToolOptionsArrows::setIcon(UBApplication::mainWindow->actionAttachment, icon);
    UBApplication::mainWindow->actionAttachment->setToolTip(hyperlink ? tr("Add a hyperlink to a web page")
                                                                       : tr("Attach a file to the page"));
}

// ---------------------------------------------------------------------------
// Spotlight and magnifier

UBSpotlightOptionsPanel::UBSpotlightOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Spotlight"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mMode = new UBSegmentedChoice({ tr("Spotlight"), tr("Magnifier") }, this);
    mMode->setIcons({ optionIcon(UBOptionIcon::Spotlight), optionIcon(UBOptionIcon::Magnifier) });
    mMode->setToolTip(tr("Spotlight: darkens all but a circle around the pointer. Magnifier: magnifies what is under the pointer."));
    addRow(mMode);

    addSection(tr("Radius"));
    mRadius = new UBValueSlider({ 40, 80, 120, 150, 200, 300, 400 }, tr(" px"), 0, this);
    addRow(mRadius);

    addSection(tr("Darkness"));
    mDarknessTitle = findChildren<QLabel*>("section").last();
    mDarkness = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
    addRow(mDarkness);

    addSection(tr("Zoom"));
    mZoomTitle = findChildren<QLabel*>("section").last();
    mZoom = new UBValueSlider({ 150, 200, 300, 400, 600 }, tr(" %"), 0, this);
    addRow(mZoom);

    connect(mMode, &UBSegmentedChoice::choiceChanged, this, &UBSpotlightOptionsPanel::onModeClicked);
    connect(UBDrawingController::drawingController(), &UBDrawingController::spotlightShownChanged,
            this, &UBSpotlightOptionsPanel::showWhetherShown);
    connect(mRadius, &UBValueSlider::valueChanged, this, [settings](qreal radius) {
        settings->spotlightRadius->set(qRound(radius));
    });
    connect(mDarkness, &UBValueSlider::valueChanged, this, [settings](qreal darkness) {
        settings->spotlightDarkness->set(darkness / 100);
    });
    connect(mZoom, &UBValueSlider::valueChanged, this, [settings](qreal zoom) {
        settings->magnifierZoom->set(zoom / 100);
    });

    refresh();
}

void UBSpotlightOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    showWhetherShown();
    mRadius->setValue(settings->spotlightRadius->get().toInt());
    mDarkness->setValue(settings->spotlightDarkness->get().toDouble() * 100);
    mZoom->setValue(settings->magnifierZoom->get().toDouble() * 100);
    showModeOptions();
}

void UBSpotlightOptionsPanel::updateToolIcon()
{
    // The tool shows a flashlight or a magnifying glass, as chosen.
    const bool magnifier = UBSettings::settings()->spotlightMode->get().toInt() == 1;
    QIcon icon;
    icon.addFile(magnifier ? ":/images/stylusPalette/magnifier.svg" : ":/images/stylusPalette/spotlight.svg",
                 QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(magnifier ? ":/images/stylusPalette/magnifierOn.svg" : ":/images/stylusPalette/spotlightOn.svg",
                 QSize(), QIcon::Normal, QIcon::On);
    UBToolOptionsArrows::setIcon(UBApplication::mainWindow->actionSpotlight, icon);
}

void UBSpotlightOptionsPanel::onModeClicked(int mode)
{
    // Spotlight and Magnifier are switches: a click turns one on, or the
    // other one into it, and a click on the one that is on turns it off.
    UBSettings* settings = UBSettings::settings();
    UBDrawingController* drawingController = UBDrawingController::drawingController();
    const bool turnOff = drawingController->isSpotlightShown() && settings->spotlightMode->get().toInt() == mode;

    settings->spotlightMode->set(mode);
    drawingController->setSpotlightShown(!turnOff);
    showWhetherShown();
    updateToolIcon();
}

void UBSpotlightOptionsPanel::showWhetherShown()
{
    // Only the spotlight or magnifier that is on looks chosen.
    if (UBDrawingController::drawingController()->isSpotlightShown())
    {
        mMode->setChoice(UBSettings::settings()->spotlightMode->get().toInt());
    }
    else
    {
        mMode->clearChoice();
    }

    showModeOptions();
}

void UBSpotlightOptionsPanel::showModeOptions()
{
    // The darkness is for the spotlight, the zoom for the magnifier.
    const bool magnifier = UBSettings::settings()->spotlightMode->get().toInt() == 1;
    mDarknessTitle->setVisible(!magnifier);
    mDarkness->setVisible(!magnifier);
    mZoomTitle->setVisible(magnifier);
    mZoom->setVisible(magnifier);
}

// ---------------------------------------------------------------------------
// Shapes

static QString shapeTitle(int tool)
{
    switch (tool)
    {
        case UBStylusTool::Rectangle:
            return UBShapeOptionsPanel::tr("Shapes");
        case UBStylusTool::Ellipse:
            return UBShapeOptionsPanel::tr("Ellipse");
        case UBStylusTool::Arrow:
            return UBShapeOptionsPanel::tr("Arrow");
        case UBStylusTool::Highlight:
            return UBShapeOptionsPanel::tr("Text highlight");
        default:
            return UBShapeOptionsPanel::tr("Underline");
    }
}

UBShapeOptionsPanel::UBShapeOptionsPanel(int tool, QWidget* parent)
    : UBToolOptionsPanel(shapeTitle(tool), parent)
    , mSettings(UBSettings::settings()->shapeSettings(tool))
    , mTool(tool)
{
    const bool closedShape = (tool == UBStylusTool::Rectangle || tool == UBStylusTool::Ellipse);
    const bool isArrow = (tool == UBStylusTool::Arrow);
    const bool isUnderline = (tool == UBStylusTool::Underline);
    const bool isHighlight = (tool == UBStylusTool::Highlight);

    // The shapes tool draws the shape chosen here.
    if (tool == UBStylusTool::Rectangle)
    {
        mShapes = new UBShapeChoice(this);
        mShapes->setToolTip(tr("The shape to draw. The polyline: a click for each corner, any key to end it, Escape to drop it."));
        addRow(mShapes);
        connect(mShapes, &UBShapeChoice::choiceChanged, this, [](int kind) {
            UBSettings::settings()->shapeKind->set(kind);
        });
    }

    if (closedShape)
    {
        mColorTarget = new UBSegmentedChoice({ tr("Border"), tr("Fill") }, this);
        mColorTarget->setIcons({ optionIcon(UBOptionIcon::Border), optionIcon(UBOptionIcon::Fill) }, Qt::ToolButtonTextBesideIcon);
        addRow(mColorTarget);
    }

    mStrokeColors = new UBColorSwatches(isHighlight ? markerColors() : penColors(), false, this);
    addRow(mStrokeColors);

    if (closedShape)
    {
        mFillColors = new UBColorSwatches(penColors(), true, this);
        addRow(mFillColors);
    }

    if (isHighlight)
    {
        addSection(tr("Height"));
        mWidth = new UBValueSlider({ 8, 12, 16, 20, 30, 40, 60 }, tr(" pt"), 0, this);
    }
    else
    {
        mWidth = new UBValueSlider({ 1, 2, 3, 6, 10, 50 }, tr(" pt"), 1, this);
    }

    addRow(mWidth);

    if (!isUnderline && !isHighlight)
    {
        addSection(isArrow ? tr("Line style") : tr("Border styles"));
        mDash = new UBPictureChoice(UBPictureChoice::Kind::Dash, 5, this);
        addRow(mDash);
    }

    if (closedShape)
    {
        mBorderStyle = new UBPictureChoice(UBPictureChoice::Kind::BorderStyle, 2, this);
        addRow(mBorderStyle);
    }

    if (isArrow)
    {
        addSection(tr("Line start"));
        mStartEnding = new UBPictureChoice(UBPictureChoice::Kind::StartEnding, 6, this);
        addRow(mStartEnding);

        addSection(tr("Line end"));
        mEndEnding = new UBPictureChoice(UBPictureChoice::Kind::EndEnding, 6, this);
        addRow(mEndEnding);

        mTapered = new QCheckBox(tr("Tapered"), this);
        mTapered->setToolTip(tr("The line grows from its start to the tip of the arrow."));
        addRow(mTapered);
    }

    if (isHighlight)
    {
        mFitText = new QCheckBox(tr("Fit to the text size"), this);
        mFitText->setToolTip(tr("On a PDF, the band takes the height of the line of text, with a little margin."));
        mFitText->setChecked(UBSettings::settings()->highlightFitText->get().toBool());
        addRow(mFitText);
        connect(mFitText, &QCheckBox::toggled, this, [](bool on) {
            UBSettings::settings()->highlightFitText->set(on);
        });
    }

    if (!isUnderline)
    {
        addSection(tr("Opacity"));
        mOpacity = new UBValueSlider(opacityMarks(), tr(" %"), 0, this);
        addRow(mOpacity);
    }

    UBSettings::ShapeSettings* settings = mSettings;

    if (mColorTarget)
    {
        connect(mColorTarget, &UBSegmentedChoice::choiceChanged, this, &UBShapeOptionsPanel::showColorTarget);
    }

    connect(mStrokeColors, &UBColorSwatches::colorChosen, this, [settings](const QColor& color) {
        settings->strokeColor->set(colorName(color));
    });

    if (mFillColors)
    {
        connect(mFillColors, &UBColorSwatches::colorChosen, this, [settings](const QColor& color) {
            settings->fillColor->set(colorName(color));
        });
    }

    connect(mWidth, &UBValueSlider::valueChanged, this, [settings](qreal width) {
        settings->width->set(width);
    });

    if (mDash)
    {
        connect(mDash, &UBPictureChoice::choiceChanged, this, [settings](int dash) {
            settings->dash->set(dash);
        });
    }

    if (mBorderStyle)
    {
        connect(mBorderStyle, &UBPictureChoice::choiceChanged, this, [settings](int style) {
            settings->borderStyle->set(style);
        });
    }

    if (mStartEnding)
    {
        connect(mStartEnding, &UBPictureChoice::choiceChanged, this, [settings](int ending) {
            settings->startEnding->set(ending);
        });
        connect(mEndEnding, &UBPictureChoice::choiceChanged, this, [settings](int ending) {
            settings->endEnding->set(ending);
        });
    }

    if (mOpacity)
    {
        connect(mOpacity, &UBValueSlider::valueChanged, this, [settings](qreal opacity) {
            settings->opacity->set(opacity / 100);
        });
    }

    if (mTapered)
    {
        connect(mTapered, &QCheckBox::toggled, this, [settings](bool on) {
            settings->tapered->set(on);
        });
    }

    refresh();
    showColorTarget(0);
}

void UBShapeOptionsPanel::refresh()
{
    if (mShapes)
    {
        mShapes->setChoice(UBSettings::settings()->shapeKind->get().toInt());
    }

    mStrokeColors->setCurrentColor(settingColor(mSettings->strokeColor));

    if (mFillColors)
    {
        mFillColors->setCurrentColor(settingColor(mSettings->fillColor));
    }

    mWidth->setValue(mSettings->width->get().toDouble());

    if (mDash)
    {
        mDash->setChoice(mSettings->dash->get().toInt());
    }

    if (mBorderStyle)
    {
        mBorderStyle->setChoice(mSettings->borderStyle->get().toInt());
    }

    if (mStartEnding)
    {
        mStartEnding->setChoice(mSettings->startEnding->get().toInt());
        mEndEnding->setChoice(mSettings->endEnding->get().toInt());
    }

    if (mTapered)
    {
        mTapered->setChecked(mSettings->tapered->get().toBool());
    }

    if (mOpacity)
    {
        mOpacity->setValue(mSettings->opacity->get().toDouble() * 100);
    }
}

bool UBShapeOptionsPanel::stacksSlidersInToolbar() const
{
    return mTool == UBStylusTool::Rectangle;
}

void UBShapeOptionsPanel::layOutForToolbar()
{
    if (mTool == UBStylusTool::Rectangle)
    {
        // The shapes at the left in three rows, then the colors; Border and
        // Fill one under the other, to save width.
        mShapes->setColumns(7);
        mColorTarget->setVertical();
        mLayout->removeWidget(mColorTarget);
        mLayout->insertWidget(mLayout->indexOf(mFillColors) + 1, mColorTarget);
        return;
    }

    if (mTool != UBStylusTool::Arrow)
    {
        return;
    }

    // In the toolbar each choice of the line shows only the chosen picture
    // and drops the others down, with a short caption.
    for (UBPictureChoice* choice : { mDash, mStartEnding, mEndEnding })
    {
        choice->setDropDown();
    }

    const QMap<QString, QString> captions = { { tr("Line style"), tr("Line") }, { tr("Line start"), tr("Start") },
                                              { tr("Line end"), tr("End") } };

    for (QLabel* label : findChildren<QLabel*>("section"))
    {
        if (captions.contains(label->text()))
        {
            label->setToolTip(label->text());
            label->setText(captions.value(label->text()));
        }
        else if (label->text() == tr("Opacity"))
        {
            label->hide();
        }
    }

    // Tapered and the opacity at the left, before the colors.
    mLayout->removeWidget(mTapered);
    mLayout->removeWidget(mOpacity);
    mLayout->insertWidget(0, mTapered);
    mLayout->insertWidget(1, mOpacity);
}

void UBShapeOptionsPanel::showColorTarget(int target)
{
    if (!mColorTarget)
    {
        return;
    }

    mColorTarget->setChoice(target);
    mStrokeColors->setVisible(target == 0);
    mFillColors->setVisible(target == 1);
}

// ---------------------------------------------------------------------------
// Text

UBTextOptionsPanel::UBTextOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Text"), parent)
{
    UBSettings* settings = UBSettings::settings();

    mFont = new QFontComboBox(this);
    addRow(mFont);

    mSize = new UBValueSlider({ 8, 12, 16, 24, 36, 48, 72 }, tr(" pt"), 0, this);
    addRow(mSize);

    auto styleRow = new QWidget(this);
    auto styleLayout = new QHBoxLayout(styleRow);
    styleLayout->setContentsMargins(0, 0, 0, 0);

    mBold = new QToolButton(styleRow);
    mBold->setText(tr("B"));
    mBold->setToolTip(tr("Bold"));
    mBold->setCheckable(true);
    QFont boldFont = mBold->font();
    boldFont.setBold(true);
    mBold->setFont(boldFont);

    mItalic = new QToolButton(styleRow);
    mItalic->setText(tr("I"));
    mItalic->setToolTip(tr("Italic"));
    mItalic->setCheckable(true);
    QFont italicFont = mItalic->font();
    italicFont.setItalic(true);
    mItalic->setFont(italicFont);

    styleLayout->addWidget(mBold);
    styleLayout->addWidget(mItalic);
    styleLayout->addStretch();
    addRow(styleRow);

    addSection(tr("Color"));
    mColors = new UBColorSwatches(penColors(), false, this);
    addRow(mColors);

    // The lists, for the text box being written. The buttons take no
    // focus, so that the text box keeps its cursor.
    const QList<std::tuple<QString, UBOptionIcon, UBGraphicsTextItem::ListStyle>> lists = {
        { tr("Checklist"), UBOptionIcon::Checklist, UBGraphicsTextItem::ListStyle::Checklist },
        { tr("Bullets"), UBOptionIcon::Bullets, UBGraphicsTextItem::ListStyle::Bullets },
        { tr("Numbers"), UBOptionIcon::Numbers, UBGraphicsTextItem::ListStyle::Numbers },
    };

    for (const auto& list : lists)
    {
        auto button = new QToolButton(this);
        button->setText(std::get<0>(list));
        button->setIcon(optionIcon(std::get<1>(list)));
        button->setIconSize(QSize(34, 19));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setFocusPolicy(Qt::NoFocus);
        button->setToolTip(tr("Puts the lines of the text box being written in a list, or takes them out of it"));
        addRow(button);

        const UBGraphicsTextItem::ListStyle style = std::get<2>(list);
        connect(button, &QToolButton::clicked, this, [style]() {
            UBGraphicsTextItem* textBox = UBTextBoxes::current();

            if (!textBox)
            {
                UBApplication::showMessage(tr("Click in a text box first, then on the list."));
                return;
            }

            textBox->toggleList(style);
        });
    }

    // A table, in the text box being written or in a new one.
    auto table = new QToolButton(this);
    table->setText(tr("Table"));
    table->setIcon(optionIcon(UBOptionIcon::Table));
    table->setIconSize(QSize(34, 19));
    table->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    table->setFocusPolicy(Qt::NoFocus);
    table->setToolTip(tr("Asks how many rows and columns, then puts a table in the text box being written, or in a new one"));
    addRow(table);
    connect(table, &QToolButton::clicked, this, [this]() { UBTextBoxes::insertTable(window()); });

    // Dictation into the text box being written.
    auto dictate = new QToolButton(this);
    dictate->setText(tr("Dictate"));
    dictate->setIcon(optionIcon(UBOptionIcon::Microphone));
    dictate->setIconSize(QSize(34, 19));
    dictate->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    dictate->setFocusPolicy(Qt::NoFocus);
    dictate->setToolTip(tr("Makes a text box and writes in it what you say (the dictation of Windows, Windows + H). "
                           "In a text box, the microphone left of F does the same."));
    addRow(dictate);
    connect(dictate, &QToolButton::clicked, this, []() { UBDictation::start(); });

    // The other way of dictation: an audio already recorded, written by an AI.
    auto transcribe = new QToolButton(this);
    transcribe->setText(tr("Transcribe"));
    transcribe->setIcon(QIcon(":/images/stylusPalette/ai.svg"));
    transcribe->setIconSize(QSize(34, 19));
    transcribe->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    transcribe->setFocusPolicy(Qt::NoFocus);
    transcribe->setToolTip(tr("Asks for an audio and gives it to an AI on the web, which writes what is said"));
    addRow(transcribe);
    connect(transcribe, &QToolButton::clicked, this, [this]() {
        UBAiPanel::startAction(UBAiPanel::Action::Transcription, window());
    });

    connect(mFont, &QFontComboBox::currentFontChanged, this, [settings](const QFont& font) {
        settings->setFontFamily(font.family());
    });
    connect(mSize, &UBValueSlider::valueChanged, this, [settings](qreal size) {
        settings->setFontPointSize(qRound(size));
    });
    connect(mBold, &QToolButton::toggled, this, [settings](bool bold) {
        settings->setBoldFont(bold);
    });
    connect(mItalic, &QToolButton::toggled, this, [settings](bool italic) {
        settings->setItalicFont(italic);
    });
    connect(mColors, &UBColorSwatches::colorChosen, this, [](const QColor& color) {
        UBGraphicsTextItem::lastUsedTextColor = color;
    });

    refresh();
}

void UBTextOptionsPanel::refresh()
{
    UBSettings* settings = UBSettings::settings();

    const QSignalBlocker fontBlocker(mFont);
    const QSignalBlocker boldBlocker(mBold);
    const QSignalBlocker italicBlocker(mItalic);

    mFont->setCurrentFont(QFont(settings->fontFamily()));
    mSize->setValue(settings->fontPointSize());
    mBold->setChecked(settings->isBoldFont());
    mItalic->setChecked(settings->isItalicFont());
    mColors->setCurrentColor(UBGraphicsTextItem::lastUsedTextColor);
}

// ---------------------------------------------------------------------------
// Notes

UBStampOptionsPanel::UBStampOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Stamp"), parent)
{
    // Remove, beside the title: a click on a stamp then takes it away.
    mRemove = new QToolButton(this);
    mRemove->setText(tr("Remove"));
    mRemove->setToolTip(tr("Click a stamp to take it away"));
    mRemove->setCheckable(true);
    mRemove->setStyleSheet("QToolButton { color: #f0f0f0; background: #3a404b; border: 1px solid #555c68; border-radius: 4px; padding: 3px 10px; }"
                           "QToolButton:checked { background: #c0392b; border-color: #e5533d; }");

    auto header = new QWidget(this);
    auto headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->removeWidget(mTitle);
    headerLayout->addWidget(mTitle);
    headerLayout->addStretch(1);
    headerLayout->addWidget(mRemove);
    mLayout->insertWidget(0, header);

    connect(mRemove, &QToolButton::toggled, this, [this](bool removing) {
        UBStamps::setRemoving(removing);

        if (removing)
        {
            UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Stamp);
        }

        refresh();
    });

    auto gallery = new QWidget(this);
    mGallery = new QGridLayout(gallery);
    mGallery->setContentsMargins(0, 0, 0, 0);
    mGallery->setSpacing(6);

    for (int i = 0; i < UBStamps::count(); ++i)
    {
        auto button = new QToolButton(gallery);
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIconSize(QSize(150, 50));
        button->setText(UBStamps::name(i));
        button->setToolTip(UBStamps::name(i));
        button->setIcon(iconFor(i));
        button->setStyleSheet("QToolButton { background: #ffffff; color: #222222; border: 1px solid #c9ced6; border-radius: 4px; padding: 4px; }"
                              "QToolButton:hover { border-color: #f6c343; }"
                              "QToolButton:checked { background: #fff4cf; border: 2px solid #f6c343; }");
        mGallery->addWidget(button, i / 3, i % 3);
        mStamps << button;

        connect(button, &QToolButton::clicked, this, [this, i]() {
            if (UBStamps::isCustom(i))
            {
                chooseCustomPicture();
            }
            else
            {
                choose(i);
            }
        });
    }

    addRow(gallery);
    refresh();
}

void UBStampOptionsPanel::refresh()
{
    const int current = UBStamps::current();
    const bool removing = UBStamps::isRemoving();

    {
        const QSignalBlocker blocker(mRemove);
        mRemove->setChecked(removing);
    }

    for (int i = 0; i < mStamps.size(); ++i)
    {
        // While removing, no stamp is the one to put.
        mStamps.at(i)->setAutoExclusive(!removing);
        mStamps.at(i)->setChecked(!removing && i == current);
        mStamps.at(i)->setAutoExclusive(true);
    }

    mStamps.first()->setIcon(iconFor(0));
}

void UBStampOptionsPanel::layOutForToolbar()
{
    // One row of small pictures, their names as tooltips.
    for (int i = 0; i < mStamps.size(); ++i)
    {
        QToolButton* button = mStamps.at(i);
        mGallery->removeWidget(button);
        mGallery->addWidget(button, 0, i);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setIconSize(QSize(72, 24));
    }

    mGallery->setSpacing(3);
}

void UBStampOptionsPanel::choose(int index)
{
    UBStamps::setCurrent(index);
    UBStamps::setRemoving(false);

    // The stamp is put with the stamp tool, which the choice selects.
    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Stamp);
    refresh();
}

void UBStampOptionsPanel::chooseCustomPicture()
{
    const QString current = UBStamps::customFile();
    const QString file = QFileDialog::getOpenFileName(window(), tr("Picture of the custom stamp"),
        current.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) : current,
        tr("PNG pictures (*.png)"));

    if (!file.isEmpty())
    {
        UBStamps::setCustomFile(file);
    }

    if (UBStamps::customFile().isEmpty())
    {
        // Nothing to stamp with: the previous choice stays.
        refresh();
        return;
    }

    choose(0);
}

QIcon UBStampOptionsPanel::iconFor(int index) const
{
    QImage picture = UBStamps::image(index);

    if (picture.isNull())
    {
        // The custom stamp before a picture is chosen: a dashed frame with
        // a large plus at the top and its name below, as large as it fits,
        // also in the toolbar where names are hidden.
        picture = QImage(UBStamps::pageSize() * 2, QImage::Format_ARGB32_Premultiplied);
        picture.fill(Qt::transparent);
        QPainter painter(&picture);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#8a9099"), 6, Qt::DashLine));
        const QRectF frame = QRectF(picture.rect()).adjusted(6, 6, -6, -6);
        painter.drawRoundedRect(frame, 20, 20);

        const QRectF plusArea(frame.left(), frame.top() + frame.height() * 0.04, frame.width(), frame.height() * 0.4);
        const QRectF textArea(frame.left() + 8, plusArea.bottom(), frame.width() - 16, frame.bottom() - plusArea.bottom() - 4);

        // A thick plus, drawn rather than written, to fill its place.
        const qreal arm = plusArea.height() * 0.75;
        const QPointF center = plusArea.center();
        painter.setPen(QPen(QColor("#5b6270"), arm * 0.22, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(center.x() - arm / 2, center.y()), QPointF(center.x() + arm / 2, center.y()));
        painter.drawLine(QPointF(center.x(), center.y() - arm / 2), QPointF(center.x(), center.y() + arm / 2));

        // The name on one line under the plus, as large as it fits.
        const QStringList lines{ UBStamps::name(index).toUpper() };
        const qreal lineHeight = textArea.height() / lines.size();

        QFont font = painter.font();
        font.setBold(true);
        int size = qRound(lineHeight * 0.85);
        font.setPixelSize(size);

        auto widest = [&lines](const QFont& f) {
            qreal width = 0;

            for (const QString& line : lines)
            {
                width = qMax(width, QFontMetricsF(f).horizontalAdvance(line));
            }

            return width;
        };

        while (size > 8 && widest(font) > textArea.width())
        {
            font.setPixelSize(--size);
        }

        painter.setFont(font);
        painter.setPen(QColor("#4a5160"));

        for (int i = 0; i < lines.size(); ++i)
        {
            painter.drawText(QRectF(textArea.left(), textArea.top() + i * lineHeight, textArea.width(), lineHeight),
                             Qt::AlignCenter, lines.at(i));
        }
    }

    return QIcon(QPixmap::fromImage(picture));
}

UBSelectorOptionsPanel::UBSelectorOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Select"), parent)
{
    // Two tiles, the drawing above the name, as for the laser.
    mMode = new UBSegmentedChoice({ tr("Arrow"), tr("Lasso") }, this);
    mMode->setIcons({ optionIcon(UBOptionIcon::SelectArrow), optionIcon(UBOptionIcon::SelectLasso) });
    mMode->setToolTip(tr("Arrow: select with a rectangle. Lasso: select by drawing a free line around the objects."));
    addRow(mMode);

    mHint = new QLabel(this);
    mHint->setStyleSheet("QLabel { color: #9aa3b2; }");
    addRow(mHint);

    connect(mMode, &UBSegmentedChoice::choiceChanged, this, [](int mode) {
        UBDrawingController::drawingController()->setLassoSelection(mode == 1);
    });
    connect(UBDrawingController::drawingController(), &UBDrawingController::lassoSelectionChanged,
            this, &UBSelectorOptionsPanel::refresh);

    refresh();
}

void UBSelectorOptionsPanel::refresh()
{
    const bool lasso = UBDrawingController::drawingController()->isLassoSelection();
    mMode->setChoice(lasso ? 1 : 0);
    mHint->setText(lasso ? tr("Draw a line around what to select.") : tr("Drag a rectangle, or over a PDF to copy its text."));
}

UBCommentOptionsPanel::UBCommentOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Quick note"), parent)
{
    auto buttons = new QWidget(this);
    auto row = new QHBoxLayout(buttons);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(4);

    auto addButton = [this, buttons, row](const QString& text, const QString& badge) {
        // The note icon with a small sign, like the comment buttons of PDF readers.
        QPixmap picture(QSize(64, 64));
        picture.fill(Qt::transparent);
        {
            QPainter painter(&picture);
            painter.setRenderHint(QPainter::Antialiasing);
            QIcon(":/images/marks/comment.svg").paint(&painter, QRect(4, 4, 48, 48));

            if (!badge.isEmpty())
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor("#3fa34d"));
                painter.drawEllipse(QRectF(32, 32, 30, 30));
                painter.setPen(Qt::white);
                QFont font = painter.font();
                font.setPixelSize(24);
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(QRectF(32, 30, 30, 30), Qt::AlignCenter, badge);
            }
        }

        auto button = new QToolButton(buttons);
        button->setIcon(QIcon(picture));
        button->setIconSize(QSize(30, 30));
        button->setText(text);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setStyleSheet(UBTheme::style("QToolButton { color: #e8e8e8; background: transparent; border: none; padding: 2px 6px; }"
                                             "QToolButton:hover { background: #3a404b; border-radius: 4px; }"
                                             "QToolButton:checked { background: #4a5260; border-radius: 4px; }"));
        row->addWidget(button);
        Q_UNUSED(this)
        return button;
    };

    auto previous = addButton(tr("Previous"), QStringLiteral("\u2039"));
    auto next = addButton(tr("Next"), QStringLiteral("\u203A"));
    auto list = addButton(tr("List of notes"), QString());
    mShow = addButton(tr("Show"), QStringLiteral("\u25C9"));
    mShow->setCheckable(true);
    mShow->setToolTip(tr("Show or hide the quick notes of the pages"));
    auto merge = addButton(tr("Merge notes"), QStringLiteral("\u2193"));
    merge->setToolTip(tr("Put the quick notes into the pages, as pictures"));

    addRow(buttons);

    connect(previous, &QToolButton::clicked, this, [this]() { UBComments::step(-1, window()); });
    connect(next, &QToolButton::clicked, this, [this]() { UBComments::step(+1, window()); });
    connect(list, &QToolButton::clicked, this, [this]() { UBComments::showList(window()); });
    connect(mShow, &QToolButton::toggled, this, [](bool shown) { UBComments::setShown(shown); });
    connect(merge, &QToolButton::clicked, this, [this]() { UBComments::merge(window()); });

    refresh();
}

void UBCommentOptionsPanel::refresh()
{
    const QSignalBlocker blocker(mShow);
    mShow->setChecked(UBGraphicsCommentItem::areShown());
}

UBNoteOptionsPanel::UBNoteOptionsPanel(QWidget* parent)
    : UBToolOptionsPanel(tr("Note"), parent)
{
    addSection(tr("Marker color"));
    mColors = new UBColorSwatches(UBColorSwatches::standardColors(), false, this);
    addRow(mColors);

    connect(mColors, &UBColorSwatches::colorChosen, this, [](const QColor& color) {
        UBSettings::settings()->noteColor->set(color.name());
    });

    refresh();
}

void UBNoteOptionsPanel::refresh()
{
    mColors->setCurrentColor(QColor(UBSettings::settings()->noteColor->get().toString()));
}
