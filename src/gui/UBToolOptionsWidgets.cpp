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

#include "UBToolOptionsWidgets.h"

#include <QPointer>

#include "domain/UBShapeGeometry.h"
#include "gui/UBEyedropper.h"

#include <cmath>

#include <QButtonGroup>
#include <QColorDialog>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QSlider>
#include <QToolButton>
#include <QtMath>

#include "core/UB.h"
#include "gui/UBTheme.h"

static constexpr int sSwatchSize = 24;
static constexpr int sSliderSteps = 1000;

QString toolOptionsStyleSheet()
{
    return UBTheme::style(QStringLiteral(
        "UBToolOptionsPanel { background: #20242b; }"
        "QLabel { color: #e8e8e8; background: transparent; }"
        "QLabel#title { font-weight: bold; font-size: 15px; }"
        "QLabel#section, QLabel#caption { color: #9aa3b2; }"
        "QToolButton { background: #2c313a; color: #e8e8e8; border: 1px solid #3a404b; border-radius: 6px; padding: 4px; }"
        "QToolButton:hover { border-color: #6b7383; }"
        "QToolButton:checked { background: #3d4452; border: 2px solid #f0f0f0; }"
        "QToolButton#swatch { border-radius: 4px; padding: 0px; }"
        "QToolButton#swatch:checked { border: 2px solid #f0f0f0; }"
        "QComboBox { background: #111317; color: #f0f0f0; border: 1px solid #3a404b; border-radius: 4px; padding: 2px 6px; min-width: 64px; }"
        "QComboBox QAbstractItemView { background: #111317; color: #f0f0f0; selection-background-color: #3d4452; }"
        "QSlider::groove:horizontal { height: 4px; background: #4a5160; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: #f0f0f0; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: #ffffff; width: 16px; height: 16px; margin: -6px 0; border-radius: 8px; }"
        "QCheckBox { color: #e8e8e8; }"
        "QFontComboBox { min-width: 140px; }"));
}

static QIcon eyedropperIcon()
{
    // A pipette, tilted, with a drop at its tip.
    QPixmap pixmap(48, 48);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(24, 24);
    painter.rotate(45);

    // Dark under light, so that it shows on dark and light buttons.
    for (const auto& pen : { QPair<QColor, qreal>(QColor("#1c1d20"), 7), QPair<QColor, qreal>(QColor("#f0f0f0"), 3) })
    {
        painter.setPen(QPen(pen.first, pen.second, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(pen.first == QColor("#f0f0f0") ? pen.first : QColor("#1c1d20"));
        painter.drawLine(QPointF(0, -4), QPointF(0, 18));
        painter.drawRoundedRect(QRectF(-6, -20, 12, 14), 4, 4);
        painter.drawLine(QPointF(-8, -6), QPointF(8, -6));
    }

    return QIcon(pixmap);
}

static QIcon otherColorIcon(int size)
{
    // A wheel of all the colors, for any other color.
    const qreal ratio = 2;
    QPixmap pixmap(QSize(size, size) * ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QConicalGradient wheel(size / 2.0, size / 2.0, 0);

    for (int i = 0; i <= 6; ++i)
    {
        wheel.setColorAt(i / 6.0, QColor::fromHsvF(std::fmod(i / 6.0, 1.0), 0.85, 1.0));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(wheel);
    painter.drawEllipse(QRectF(0, 0, size, size).adjusted(2.5, 2.5, -2.5, -2.5));

    return QIcon(pixmap);
}

static QString swatchStyle(const QColor& color)
{
    if (!color.isValid())
    {
        // "None" swatch: a white square crossed by a red line.
        return QStringLiteral("QToolButton#swatch { background: qlineargradient(x1:0, y1:1, x2:1, y2:0, "
                              "stop:0 #f0f0f0, stop:0.46 #f0f0f0, stop:0.5 #e02020, stop:0.54 #f0f0f0, stop:1 #f0f0f0); }");
    }

    return QStringLiteral("QToolButton#swatch { background: %1; }").arg(color.name());
}

UBColorSwatches::UBColorSwatches(const QList<QColor>& colors, bool offerNone, QWidget* parent)
    : QWidget(parent)
    , mColors(colors)
{
    if (offerNone)
    {
        mColors.prepend(QColor());
    }

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    for (const QColor& color : std::as_const(mColors))
    {
        auto swatch = new QToolButton(this);
        swatch->setObjectName("swatch");
        swatch->setCheckable(true);
        swatch->setFixedSize(sSwatchSize, sSwatchSize);
        swatch->setStyleSheet(swatchStyle(color));
        swatch->setToolTip(color.isValid() ? color.name() : tr("No color"));
        layout->addWidget(swatch);
        mSwatches << swatch;

        connect(swatch, &QToolButton::clicked, this, [this, color]() {
            selectSwatch(color);
            emit colorChosen(color);
        });
    }

    mOtherColorButton = new QToolButton(this);
    mOtherColorButton->setText(QStringLiteral("…"));
    mOtherColorButton->setToolTip(tr("Other color"));
    mOtherColorButton->setFixedSize(sSwatchSize + 6, sSwatchSize);
    layout->addWidget(mOtherColorButton);

    // Takes a color anywhere on the screen.
    mEyedropperButton = new QToolButton(this);
    mEyedropperButton->setIcon(eyedropperIcon());
    mEyedropperButton->setIconSize(QSize(sSwatchSize - 4, sSwatchSize - 4));
    mEyedropperButton->setToolTip(tr("Eyedropper: take a color on the screen"));
    mEyedropperButton->setFixedSize(sSwatchSize + 6, sSwatchSize);
    mEyedropperButton->setStyleSheet("QToolButton { padding: 0px; }");
    layout->addWidget(mEyedropperButton);
    layout->addStretch();

    connect(mOtherColorButton, &QToolButton::clicked, this, &UBColorSwatches::chooseOtherColor);
    connect(mEyedropperButton, &QToolButton::clicked, this, &UBColorSwatches::takeColorOnScreen);
}

void UBColorSwatches::takeColorOnScreen()
{
    // A popup keeps the mouse for itself: it is closed first.
    QWidget* topLevel = window();

    if (topLevel && (topLevel->windowFlags() & Qt::Popup) == Qt::Popup)
    {
        topLevel->hide();
    }

    QPointer<UBColorSwatches> swatches(this);

    UBEyedropper::pick([swatches](const QColor& color) {
        if (swatches)
        {
            swatches->selectSwatch(color);
            emit swatches->colorChosen(color);
        }
    });
}

static QIcon roundSwatchIcon(const QColor& color, int size)
{
    // A circle of the color; the chosen one has a white ring around it.
    const qreal ratio = 2;
    QIcon icon;

    for (bool chosen : { false, true })
    {
        QPixmap pixmap(QSize(size, size) * ratio);
        pixmap.setDevicePixelRatio(ratio);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF circle = QRectF(0, 0, size, size).adjusted(1.2, 1.2, -1.2, -1.2);

        if (color.isValid())
        {
            painter.setPen(QPen(QColor("#5a6170"), 0.8));
            painter.setBrush(color);
            painter.drawEllipse(circle);
        }
        else
        {
            // No color: a white circle crossed by a red line.
            painter.setPen(QPen(QColor("#5a6170"), 0.8));
            painter.setBrush(QColor("#f0f0f0"));
            painter.drawEllipse(circle);
            painter.setPen(QPen(QColor("#e02020"), 1.4));
            painter.drawLine(circle.bottomLeft() + QPointF(2, -2), circle.topRight() + QPointF(-2, 2));
        }

        if (chosen)
        {
            painter.setPen(QPen(Qt::white, 1.6));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(QRectF(0, 0, size, size).adjusted(0.8, 0.8, -0.8, -0.8));
        }

        icon.addPixmap(pixmap, QIcon::Normal, chosen ? QIcon::On : QIcon::Off);
    }

    return icon;
}

void UBColorSwatches::setThreeRows()
{
    // Round swatches in three rows, within the height of the toolbar. The
    // style of the toolbar gives its buttons margins, which would shrink
    // the circles.
    const int size = 20;
    const QString flat = QStringLiteral("QToolButton, QToolButton:checked, QToolButton:hover, QToolButton:pressed "
                                        "{ background: transparent; border: none; padding: 0px; margin: 0px; }");
    QList<QWidget*> buttons;

    for (int i = 0; i < mSwatches.size(); ++i)
    {
        QToolButton* swatch = mSwatches.at(i);
        swatch->setObjectName("roundSwatch");
        swatch->setStyleSheet(flat);
        swatch->setIcon(roundSwatchIcon(mColors.at(i), size));
        swatch->setIconSize(QSize(size, size));
        buttons << swatch;
    }

    if (!mOtherColorButton->isHidden())
    {
        mOtherColorButton->setText(QString());
        mOtherColorButton->setIcon(otherColorIcon(size));
        mOtherColorButton->setIconSize(QSize(size, size));
        mOtherColorButton->setStyleSheet(flat);
        buttons << mOtherColorButton;
    }

    if (!mEyedropperButton->isHidden())
    {
        mEyedropperButton->setStyleSheet(flat);
        mEyedropperButton->setIconSize(QSize(size - 3, size - 3));
        buttons << mEyedropperButton;
    }

    delete layout();

    auto grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(1);
    grid->setVerticalSpacing(1);

    const int columns = (buttons.size() + 2) / 3;

    for (int i = 0; i < buttons.size(); ++i)
    {
        buttons.at(i)->setFixedSize(size, size);
        grid->addWidget(buttons.at(i), i / columns, i % columns);
    }

    grid->setColumnStretch(columns, 1);

    // Exactly three rows high, so that a taller toolbar does not spread
    // the rows until the last one is cut.
    setFixedHeight(3 * size + 2 * grid->verticalSpacing());
    setSizePolicy(sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
}

QList<QColor> UBColorSwatches::standardColors()
{
    return { QColor("#ff3b30"), QColor("#2f54ff"), QColor("#000000"), QColor("#14b85a"), QColor("#9a46f0"),
             QColor("#ff9a1f"), QColor("#12b5e8"), QColor("#ff1fd2"), QColor("#ffee1f"), QColor("#ffffff"),
             QColor("#8e8e93"), QColor("#8b5a2b"), QColor("#1a237e") };
}

void UBColorSwatches::setCurrentColor(const QColor& color)
{
    selectSwatch(color);
}

QColor UBColorSwatches::currentColor() const
{
    return mCurrentColor;
}

void UBColorSwatches::selectSwatch(const QColor& color)
{
    mCurrentColor = color;

    QColor opaque = color;

    if (opaque.isValid())
    {
        opaque.setAlpha(255);
    }

    for (int i = 0; i < mSwatches.size(); ++i)
    {
        const QColor& swatchColor = mColors.at(i);
        const bool same = (!swatchColor.isValid() && !opaque.isValid())
                || (swatchColor.isValid() && opaque.isValid() && swatchColor.rgb() == opaque.rgb());
        mSwatches.at(i)->setChecked(same);
    }
}

void UBColorSwatches::chooseOtherColor()
{
    const QColor initial = mCurrentColor.isValid() ? mCurrentColor : QColor(Qt::black);

    // A popup keeps the mouse for itself: it must be closed first, or the
    // dialog cannot be clicked.
    QWidget* topLevel = window();

    if (topLevel && (topLevel->windowFlags() & Qt::Popup) == Qt::Popup)
    {
        topLevel->hide();
    }

    // Kept above the desktop annotation layer, which stays on top of all windows.
    QColorDialog dialog(initial);
    dialog.setWindowTitle(tr("Choose a color"));
    dialog.setWindowFlag(Qt::WindowStaysOnTopHint);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const QColor color = dialog.selectedColor();

    if (color.isValid())
    {
        selectSwatch(color);
        emit colorChosen(color);
    }
}

void UBColorSwatches::hideOtherColorButton()
{
    // Only these colors: no other color, nor one taken on the screen.
    mOtherColorButton->hide();
    mEyedropperButton->hide();
}

UBValueSlider::UBValueSlider(const QList<qreal>& marks, const QString& unit, int decimals, QWidget* parent)
    : QWidget(parent)
    , mMarks(marks)
    , mUnit(unit)
    , mDecimals(decimals)
{
    mSlider = new QSlider(Qt::Horizontal, this);
    mSlider->setRange(0, sSliderSteps);
    mSlider->setMinimumWidth(170);

    mValueBox = new QComboBox(this);
    mValueBox->setEditable(true);
    mValueBox->setInsertPolicy(QComboBox::NoInsert);

    for (qreal mark : std::as_const(mMarks))
    {
        mValueBox->addItem(QString::number(mark, 'f', mDecimals) + mUnit, mark);
    }

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mSlider, 1);
    layout->addWidget(mValueBox);

    connect(mSlider, &QSlider::valueChanged, this, &UBValueSlider::onSliderMoved);
    connect(mValueBox, qOverload<int>(&QComboBox::activated), this, [this](int index) {
        setValue(mValueBox->itemData(index).toDouble());
        emit valueChanged(mValue);
    });
    connect(mValueBox->lineEdit(), &QLineEdit::editingFinished, this, &UBValueSlider::onTextEdited);
}

void UBValueSlider::setValue(qreal value)
{
    mValue = qBound(mMarks.first(), value, mMarks.last());

    const QSignalBlocker blocker(mSlider);
    mSlider->setValue(positionOf(mValue));
    showValue(mValue);
}

qreal UBValueSlider::value() const
{
    return mValue;
}

void UBValueSlider::setCompact()
{
    mSlider->setMinimumWidth(90);
    mSlider->setMaximumWidth(140);
}

void UBValueSlider::setSliderWidth(int width)
{
    mSlider->setFixedWidth(width);
}

void UBValueSlider::setStacked(const QString& caption)
{
    delete layout();

    // In a column, to take little width in the toolbar: the caption, the
    // value under it, and the slider at the bottom.
    auto column = new QVBoxLayout(this);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(1);

    if (!caption.isEmpty())
    {
        auto label = new QLabel(caption, this);
        label->setObjectName("caption");
        column->addWidget(label, 0, Qt::AlignLeft);
    }

    column->addWidget(mValueBox, 0, Qt::AlignLeft);
    column->addWidget(mSlider);

    mValueBox->setFixedHeight(20);
    mSlider->setFixedHeight(16);
    mSlider->setMinimumWidth(96);
    mSlider->setMaximumWidth(120);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedHeight(caption.isEmpty() ? 40 : 58);
}

int UBValueSlider::positionOf(qreal value) const
{
    const int intervals = mMarks.size() - 1;

    for (int i = 0; i < intervals; ++i)
    {
        if (value <= mMarks.at(i + 1))
        {
            const qreal ratio = (value - mMarks.at(i)) / (mMarks.at(i + 1) - mMarks.at(i));
            return qRound((i + ratio) * sSliderSteps / intervals);
        }
    }

    return sSliderSteps;
}

qreal UBValueSlider::valueAt(int position) const
{
    const int intervals = mMarks.size() - 1;
    const qreal scaled = qreal(position) * intervals / sSliderSteps;
    const int i = qMin(int(scaled), intervals - 1);
    const qreal ratio = scaled - i;

    return mMarks.at(i) + ratio * (mMarks.at(i + 1) - mMarks.at(i));
}

void UBValueSlider::onSliderMoved(int position)
{
    const qreal factor = std::pow(10.0, mDecimals);
    mValue = std::round(valueAt(position) * factor) / factor;
    showValue(mValue);
    emit valueChanged(mValue);
}

void UBValueSlider::onTextEdited()
{
    QString text = mValueBox->currentText();
    text.remove(mUnit.trimmed());
    text.replace(',', '.');

    bool ok = false;
    const qreal value = text.trimmed().toDouble(&ok);

    if (ok)
    {
        setValue(value);
        emit valueChanged(mValue);
    }
    else
    {
        showValue(mValue);
    }
}

void UBValueSlider::showValue(qreal value)
{
    const QSignalBlocker blocker(mValueBox);
    mValueBox->setCurrentIndex(-1);
    mValueBox->setEditText(QString::number(value, 'f', mDecimals) + mUnit);
}

UBSegmentedChoice::UBSegmentedChoice(const QStringList& labels, QWidget* parent)
    : QWidget(parent)
{
    mGroup = new QButtonGroup(this);
    mGroup->setExclusive(true);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    for (int i = 0; i < labels.size(); ++i)
    {
        auto button = new QToolButton(this);
        button->setText(labels.at(i));
        button->setCheckable(true);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mGroup->addButton(button, i);
        layout->addWidget(button);
    }

    connect(mGroup, &QButtonGroup::idClicked, this, &UBSegmentedChoice::choiceChanged);
}

void UBSegmentedChoice::setChoice(int index)
{
    if (auto button = mGroup->button(index))
    {
        button->setChecked(true);
    }
}

int UBSegmentedChoice::choice() const
{
    return mGroup->checkedId();
}

void UBSegmentedChoice::clearChoice()
{
    // An exclusive group keeps one button checked.
    mGroup->setExclusive(false);

    for (QAbstractButton* button : mGroup->buttons())
    {
        button->setChecked(false);
    }

    mGroup->setExclusive(true);
}

void UBSegmentedChoice::setIcons(const QList<QIcon>& icons, Qt::ToolButtonStyle style)
{
    const QList<QAbstractButton*> buttons = mGroup->buttons();

    for (int i = 0; i < buttons.size() && i < icons.size(); ++i)
    {
        auto button = static_cast<QToolButton*>(buttons.at(i));
        button->setIcon(icons.at(i));
        button->setToolButtonStyle(style);

        if (mSmall)
        {
            button->setIconSize(QSize(32, 18));
        }
        else if (mCompact)
        {
            button->setIconSize(style == Qt::ToolButtonTextUnderIcon ? QSize(34, 19) : QSize(28, 16));
        }
        else
        {
            button->setIconSize(style == Qt::ToolButtonTextUnderIcon ? QSize(48, 28) : QSize(34, 20));
        }
    }

    if (!mSmall)
    {
        equalizeWidths();
    }
}

void UBSegmentedChoice::equalizeWidths()
{
    // Tiles of the same size, like in Drawboard PDF.
    int widest = 0;

    for (QAbstractButton* button : mGroup->buttons())
    {
        button->setMinimumWidth(0);
        widest = qMax(widest, button->sizeHint().width());
    }

    for (QAbstractButton* button : mGroup->buttons())
    {
        button->setMinimumWidth(widest);
    }
}

void UBSegmentedChoice::setSmall()
{
    mSmall = true;

    for (QAbstractButton* button : mGroup->buttons())
    {
        QFont font = button->font();
        font.setPointSizeF(font.pointSizeF() * 0.85);
        button->setFont(font);
        button->setStyleSheet("QToolButton { padding: 1px 2px; }");

        if (!button->icon().isNull())
        {
            static_cast<QToolButton*>(button)->setIconSize(QSize(32, 18));
        }

        // Each as wide as its own name.
        button->setMinimumWidth(0);
    }
}

void UBSegmentedChoice::setVertical()
{
    delete layout();

    auto column = new QVBoxLayout(this);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(2);

    for (QAbstractButton* button : mGroup->buttons())
    {
        auto toolButton = static_cast<QToolButton*>(button);
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolButton->setIconSize(QSize(18, 12));
        toolButton->setFixedHeight(24);
        toolButton->setMinimumWidth(0);
        toolButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        toolButton->setStyleSheet("QToolButton { padding: 1px 4px; }");
        column->addWidget(toolButton);
    }
}

void UBSegmentedChoice::setCompact()
{
    mCompact = true;

    for (QAbstractButton* button : mGroup->buttons())
    {
        const QString label = button->text();
        button->setToolTip(QString(label).replace(QChar('\n'), QStringLiteral(" - ")));
        button->setText(label.section('\n', 0, 0));
        button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        if (!button->icon().isNull())
        {
            // Small enough for the height of the toolbar.
            auto toolButton = static_cast<QToolButton*>(button);
            const bool under = (toolButton->toolButtonStyle() == Qt::ToolButtonTextUnderIcon);
            toolButton->setIconSize(under ? QSize(34, 19) : QSize(28, 16));
        }
    }

    equalizeWidths();
}

/**
 * Fills the outline of a stroke along the path, as wide as each kind of
 * pen draws it: even for the basic pen, thin at both ends like a pen
 * pressed harder in the middle for the pro pen, and depending on the
 * direction like a slanted nib for the calligraphy pen.
 */
static void drawStroke(QPainter& painter, const QPainterPath& path, qreal width, int type, const QColor& color)
{
    const int samples = 64;
    QList<QPointF> points;

    for (int i = 0; i <= samples; ++i)
    {
        points << path.pointAtPercent(qreal(i) / samples);
    }

    QPolygonF left;
    QPolygonF right;
    QPainterPath outline;
    outline.setFillRule(Qt::WindingFill);

    for (int i = 0; i <= samples; ++i)
    {
        const QPointF before = points.at(qMax(0, i - 1));
        const QPointF after = points.at(qMin(samples, i + 1));
        QPointF direction = after - before;
        const qreal length = std::hypot(direction.x(), direction.y());

        if (length > 0)
        {
            direction /= length;
        }

        const qreal t = qreal(i) / samples;
        qreal w = width;

        if (type == UBPenStrokeType::Pro)
        {
            const qreal angle = std::atan2(direction.y(), direction.x());
            w = width * (0.35 + 0.65 * std::sin(M_PI * t)) * (0.6 + 0.4 * std::abs(std::sin(angle - M_PI / 4)));
        }
        else if (type == UBPenStrokeType::Pencil)
        {
            w = width * 0.6;
        }
        else if (type == UBPenStrokeType::Marker)
        {
            w = width * 1.6;
        }
        else if (type == UBPenStrokeType::Calligraphy)
        {
            // A nib held at 45 degrees: thin along it, wide across it.
            const qreal angle = std::atan2(direction.y(), direction.x());
            w = width * (0.15 + 0.85 * std::abs(std::sin(angle - M_PI / 4)));
        }

        const QPointF normal(-direction.y() * w / 2, direction.x() * w / 2);
        left << points.at(i) + normal;
        right.prepend(points.at(i) - normal);

        // A nib leaves square ends, the other pens round ones.
        if ((i == 0 || i == samples) && type != UBPenStrokeType::Calligraphy)
        {
            outline.addEllipse(points.at(i), w / 2, w / 2);
        }
    }

    outline.addPolygon(left + right);
    painter.fillPath(outline, color);
}

static QPainterPath wavePath(const QRectF& box)
{
    const qreal middle = box.center().y();
    const qreal amplitude = box.height() * 0.42;

    QPainterPath wave;
    wave.moveTo(box.left(), middle);
    wave.cubicTo(box.left() + box.width() * 0.3, middle - 2 * amplitude,
                 box.left() + box.width() * 0.7, middle + 2 * amplitude,
                 box.right(), middle);
    return wave;
}

QIcon penStyleIcon(int strokeType, const QColor& color)
{
    // Drawn at twice the size, sharp on high resolution screens.
    const QSize size(48, 28);
    const qreal ratio = 2;
    QPixmap pixmap(size * ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    const QColor ink = color.isValid() ? QColor(color.rgb()) : QColor("#ff5468");
    const QColor body("#5f646c");
    const QColor dark("#3d4148");
    const QColor outline("#1c1d20");

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // The pen lies across the icon, its tip down at the left. Along x, the
    // tip is at -20 and the end at +22; across, the body is 9 high.
    painter.translate(24, 14);
    painter.rotate(-22);
    painter.scale(1.1, 1.25);
    painter.setPen(QPen(outline, 0.7));

    auto polygon = [&painter](const QColor& fill, std::initializer_list<QPointF> points) {
        painter.setBrush(fill);
        painter.drawPolygon(QPolygonF(points));
    };

    switch (strokeType)
    {
        case UBPenStrokeType::Pro:
            // Fountain pen: dark body, a band of the color, a metal nib.
            painter.setBrush(dark);
            painter.drawRoundedRect(QRectF(-6, -4.5, 28, 9), 2, 2);
            painter.setBrush(ink);
            painter.drawRect(QRectF(-6, -4.5, 3, 9));
            polygon(QColor("#e3e7ec"), { QPointF(-6, -4), QPointF(-20, 0), QPointF(-6, 4) });
            painter.drawLine(QPointF(-17, 0), QPointF(-9, 0));
            painter.setBrush(ink);
            painter.drawEllipse(QPointF(-9.5, 0), 1, 1);
            break;

        case UBPenStrokeType::Calligraphy:
            // Calligraphy pen or brush: body of the color, a flat brush tip.
            painter.setBrush(ink);
            painter.drawRoundedRect(QRectF(-4, -4, 26, 8), 2, 2);
            polygon(body, { QPointF(-4, -4.5), QPointF(-9, -3.5), QPointF(-9, 3.5), QPointF(-4, 4.5) });
            polygon(ink.darker(120), { QPointF(-9, -3.5), QPointF(-20, -1.5), QPointF(-20, 1.5), QPointF(-9, 3.5) });
            break;

        case UBPenStrokeType::Pencil:
            // Pencil: hexagonal body of the color, wood, and a graphite point.
            painter.setBrush(ink);
            painter.drawRect(QRectF(-6, -4, 26, 8));
            painter.setPen(QPen(ink.darker(140), 0.7));
            painter.drawLine(QPointF(-6, 0), QPointF(20, 0));
            painter.setPen(QPen(outline, 0.7));
            painter.setBrush(QColor("#f2a7b8"));
            painter.drawRoundedRect(QRectF(20, -4, 4, 8), 1.5, 1.5);
            polygon(QColor("#e8c58a"), { QPointF(-6, -4), QPointF(-17, -1.2), QPointF(-17, 1.2), QPointF(-6, 4) });
            polygon(QColor("#3b3b3b"), { QPointF(-17, -1.2), QPointF(-20, 0), QPointF(-17, 1.2) });
            break;

        case UBPenStrokeType::Marker:
            // Felt marker: thick grey body, a cap ring and a wide felt tip of the color.
            painter.setBrush(body);
            painter.drawRoundedRect(QRectF(-8, -5.5, 30, 11), 2.5, 2.5);
            painter.setBrush(ink);
            painter.drawRect(QRectF(-8, -5.5, 4, 11));
            painter.drawRect(QRectF(16, -5.5, 6, 11));
            polygon(QColor("#8d929a"), { QPointF(-8, -4.5), QPointF(-13, -3.5), QPointF(-13, 3.5), QPointF(-8, 4.5) });
            painter.setBrush(ink);
            painter.drawRoundedRect(QRectF(-19, -3, 7, 6), 2.5, 2.5);
            break;

        default:
            // Ballpoint pen: grey body, a band of the color, a cone and a tip of the color.
            painter.setBrush(body);
            painter.drawRoundedRect(QRectF(-8, -4, 30, 8), 2, 2);
            painter.setBrush(ink);
            painter.drawRect(QRectF(-8, -4, 4, 8));
            painter.drawRect(QRectF(18, -4, 2, 8));
            polygon(QColor("#9aa0a8"), { QPointF(-8, -4), QPointF(-18, -1), QPointF(-18, 1), QPointF(-8, 4) });
            painter.setBrush(ink);
            painter.drawEllipse(QPointF(-19, 0), 1.6, 1.6);
            break;
    }

    return QIcon(pixmap);
}

// ---------------------------------------------------------------------------

// The drawing of a shape in light grey, on the dark toolbar.
static QIcon shapeIcon(int kind)
{
    const QSize size(20, 20);
    const qreal ratio = 3; // sharp also when drawn larger, in the options window
    QPixmap pixmap(size * ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(UBTheme::color("#e8e8e8"), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    const QPolygonF outline = UBShapeGeometry::outline(kind, QRectF(2.5, 3.5, 15, 13));

    if (kind == UBShapeKind::Line || kind == UBShapeKind::Polyline)
    {
        painter.drawPolyline(outline);
    }
    else
    {
        painter.drawPolygon(outline);
    }

    painter.end();
    return QIcon(pixmap);
}

UBShapeChoice::UBShapeChoice(QWidget* parent)
    : QWidget(parent)
{
    mGroup = new QButtonGroup(this);
    mGroup->setExclusive(true);

    const QString style = QStringLiteral("QToolButton { background: transparent; border: 1px solid transparent; "
                                         "border-radius: 3px; padding: 0px; margin: 0px; }"
                                         "QToolButton:hover { border-color: #8a8f99; }"
                                         "QToolButton:checked { background: rgba(255, 255, 255, 60); border-color: #e8e8e8; }");
    const QString themed = UBTheme::style(style);

    for (int kind : UBShapeGeometry::shapeKinds())
    {
        auto button = new QToolButton(this);
        button->setIcon(shapeIcon(kind));
        button->setIconSize(QSize(20, 20));
        button->setFixedSize(22, 22);
        button->setCheckable(true);
        button->setStyleSheet(themed);
        mGroup->addButton(button, kind);
    }

    connect(mGroup, &QButtonGroup::idClicked, this, &UBShapeChoice::choiceChanged);
    setWide(10);
}

void UBShapeChoice::setChoice(int kind)
{
    if (QAbstractButton* button = mGroup->button(kind))
    {
        button->setChecked(true);
    }
}

void UBShapeChoice::setColumns(int columns)
{
    delete layout();

    auto grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(1);

    const QList<int> kinds = UBShapeGeometry::shapeKinds();

    for (int i = 0; i < kinds.size(); ++i)
    {
        QAbstractButton* button = mGroup->button(kinds.at(i));
        button->setFixedSize(22, 22);
        button->setIconSize(QSize(20, 20));
        grid->addWidget(button, i / columns, i % columns);
    }

    const int rows = (kinds.size() + columns - 1) / columns;
    setFixedSize(columns * 22 + (columns - 1), rows * 22 + (rows - 1));
}

void UBShapeChoice::setWide(int columns)
{
    delete layout();

    auto grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(3);

    const QList<int> kinds = UBShapeGeometry::shapeKinds();

    for (int i = 0; i < kinds.size(); ++i)
    {
        QAbstractButton* button = mGroup->button(kinds.at(i));
        button->setMinimumSize(34, 34);
        button->setMaximumSize(QWIDGETSIZE_MAX, 34);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setIconSize(QSize(28, 28));
        grid->addWidget(button, i / columns, i % columns);
    }

    // As wide as the window, as high as its rows.
    const int rows = (kinds.size() + columns - 1) / columns;
    setMinimumSize(columns * 34 + (columns - 1) * 3, rows * 34 + (rows - 1) * 3);
    setMaximumSize(QWIDGETSIZE_MAX, rows * 34 + (rows - 1) * 3);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

// A small four-pointed star, the sign of the AI.
static void drawSparkle(QPainter& painter, const QPointF& center, qreal radius, const QColor& color)
{
    QPainterPath star;
    star.moveTo(center.x(), center.y() - radius);
    star.quadTo(center, QPointF(center.x() + radius, center.y()));
    star.quadTo(center, QPointF(center.x(), center.y() + radius));
    star.quadTo(center, QPointF(center.x() - radius, center.y()));
    star.quadTo(center, QPointF(center.x(), center.y() - radius));
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawPath(star);
}

QIcon optionIcon(UBOptionIcon kind)
{
    // Drawn at twice the size, sharp on high resolution screens.
    const QSize size(48, 28);
    const qreal ratio = 2;
    QPixmap pixmap(size * ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    const QColor red("#ff5468");
    const QColor blue("#5b8def");
    const QRectF wave(5, 4, 38, 20);

    switch (kind)
    {
        case UBOptionIcon::BasicStroke:
            drawStroke(painter, wavePath(wave), 3.5, UBPenStrokeType::Basic, red);
            break;
        case UBOptionIcon::ProStroke:
            drawStroke(painter, wavePath(wave), 6.5, UBPenStrokeType::Pro, red);
            break;
        case UBOptionIcon::CalligraphyStroke:
            drawStroke(painter, wavePath(wave), 7, UBPenStrokeType::Calligraphy, red);
            break;
        case UBOptionIcon::Border:
            painter.setPen(QPen(blue, 2.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRectF(14, 4, 20, 20));
            break;
        case UBOptionIcon::Fill:
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            painter.drawRect(QRectF(13, 3, 22, 22));
            break;
        case UBOptionIcon::LaserTrail:
        {
            // A trail which fades towards its start.
            const QPainterPath path = wavePath(wave);
            const int segments = 12;

            for (int i = 0; i < segments; ++i)
            {
                QColor glow = red;
                glow.setAlphaF(0.15 + 0.85 * (i + 1) / segments);
                painter.setPen(QPen(glow, 3, Qt::SolidLine, Qt::RoundCap));
                painter.drawLine(path.pointAtPercent(qreal(i) / segments), path.pointAtPercent(qreal(i + 1) / segments));
            }
            break;
        }
        case UBOptionIcon::LaserPoint:
        {
            QRadialGradient halo(QPointF(24, 14), 11);
            halo.setColorAt(0, red);
            halo.setColorAt(0.45, QColor(255, 84, 104, 150));
            halo.setColorAt(1, Qt::transparent);
            painter.setPen(Qt::NoPen);
            painter.setBrush(halo);
            painter.drawEllipse(QPointF(24, 14), 11, 11);
            painter.setBrush(red);
            painter.drawEllipse(QPointF(24, 14), 4, 4);
            break;
        }
        case UBOptionIcon::Spotlight:
        {
            // A dark field with a light circle.
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 190));
            QPainterPath shade;
            shade.addRoundedRect(QRectF(4, 3, 40, 22), 4, 4);
            shade.addEllipse(QPointF(24, 14), 8, 8);
            painter.drawPath(shade);
            painter.setPen(QPen(QColor("#ffd84a"), 1.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(QPointF(24, 14), 8, 8);
            break;
        }
        case UBOptionIcon::Magnifier:
        {
            // A magnifying glass.
            painter.setPen(QPen(UBTheme::color("#e8e8e8"), 2.2));
            painter.setBrush(QColor(91, 141, 239, 90));
            painter.drawEllipse(QPointF(21, 12), 7.5, 7.5);
            painter.setPen(QPen(UBTheme::color("#e8e8e8"), 3.5, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(26.5, 17.5), QPointF(32, 23));
            break;
        }
        case UBOptionIcon::LaserFollow:
        {
            // A trail fading behind the mouse pointer, which is not pressed.
            const QPainterPath path = wavePath(QRectF(3, 6, 30, 16));
            const int pieces = 12;

            for (int i = 0; i < pieces; ++i)
            {
                QColor color = red;
                color.setAlphaF(qreal(i + 1) / pieces);
                painter.setPen(QPen(color, 3, Qt::SolidLine, Qt::RoundCap));
                painter.drawLine(path.pointAtPercent(qreal(i) / pieces), path.pointAtPercent(qreal(i + 1) / pieces));
            }

            const QPointF tip(34, 13);
            painter.setPen(QPen(QColor("#1c1d20"), 0.8));
            painter.setBrush(Qt::white);
            painter.drawPolygon(QPolygonF({ tip, tip + QPointF(0, 12), tip + QPointF(3, 9), tip + QPointF(5.5, 13.5),
                                            tip + QPointF(7, 12.8), tip + QPointF(4.8, 8.5), tip + QPointF(8.5, 8.5) }));
            break;
        }
        case UBOptionIcon::MarkerRoundTip:
        case UBOptionIcon::MarkerChiselTip:
        {
            // The mark the tip leaves, then the tip at its end: a bullet
            // tip leaves round ends, a chisel tip straight ones.
            const bool round = (kind == UBOptionIcon::MarkerRoundTip);
            QColor ink("#e6d23a");
            ink.setAlphaF(0.85);
            painter.setPen(Qt::NoPen);
            painter.setBrush(ink);

            if (round)
            {
                painter.drawRoundedRect(QRectF(3, 9, 30, 10), 5, 5);
            }
            else
            {
                painter.drawPolygon(QPolygonF({ QPointF(5, 7), QPointF(31, 7), QPointF(27, 21), QPointF(1, 21) }));
            }

            // The marker, grey with a colored body, seen from the side.
            painter.save();
            painter.translate(round ? 36 : 33, 14);
            painter.rotate(-35);
            painter.setPen(QPen(QColor("#3a3a3a"), 0.8));
            painter.setBrush(QColor("#9a9a9a"));

            if (round)
            {
                painter.drawRoundedRect(QRectF(-3, -3.5, 8, 7), 3.5, 3.5);
            }
            else
            {
                painter.drawPolygon(QPolygonF({ QPointF(-3, -1), QPointF(4, -4), QPointF(4, 4), QPointF(-3, 4) }));
            }

            painter.setBrush(QColor("#d9c32c"));
            painter.drawRect(QRectF(4, -4.5, 12, 9));
            painter.restore();
            break;
        }
        case UBOptionIcon::LaserStandard:
        case UBOptionIcon::LaserWhiteBurn:
        {
            const QPainterPath path = wavePath(wave);
            QColor glow = red;
            glow.setAlphaF(0.35);
            painter.setPen(QPen(glow, 8, Qt::SolidLine, Qt::RoundCap));
            painter.drawPath(path);
            const QColor core = (kind == UBOptionIcon::LaserWhiteBurn) ? QColor(Qt::white) : red;
            painter.setPen(QPen(core, 2.5, Qt::SolidLine, Qt::RoundCap));
            painter.drawPath(path);
            break;
        }
        case UBOptionIcon::EraserWhole:
        case UBOptionIcon::EraserPartial:
        {
            const QPainterPath path = wavePath(wave);
            const QPointF touch = path.pointAtPercent(0.5);

            if (kind == UBOptionIcon::EraserWhole)
            {
                // The whole stroke goes as soon as the eraser touches it.
                QColor ghost = red;
                ghost.setAlphaF(0.3);
                painter.setPen(QPen(ghost, 3.5, Qt::DotLine, Qt::RoundCap));
                painter.drawPath(path);
            }
            else
            {
                // Only the part under the eraser goes.
                painter.setPen(QPen(red, 3.5, Qt::SolidLine, Qt::RoundCap));
                painter.drawPath(path);
                painter.setCompositionMode(QPainter::CompositionMode_Clear);
                painter.setPen(Qt::NoPen);
                painter.setBrush(Qt::black);
                painter.drawEllipse(touch, 8, 8);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            }

            // The eraser, pink and blue, tilted on the place it touches.
            painter.save();
            painter.translate(touch + QPointF(2, -3));
            painter.rotate(-30);
            painter.setPen(QPen(QColor("#5a4a55"), 1));
            painter.setBrush(QColor("#ff9fc4"));
            painter.drawRoundedRect(QRectF(-8, -5.5, 10, 11), 2, 2);
            painter.setBrush(QColor("#6f8de8"));
            painter.drawRoundedRect(QRectF(2, -5.5, 8, 11), 2, 2);
            painter.restore();
            break;
        }
        case UBOptionIcon::ReadFromStart:
        case UBOptionIcon::ReadSelection:
        case UBOptionIcon::ReadFromClick:
        {
            // A page with lines of text; what is read is in blue.
            const QColor page = UBTheme::color("#e8e8e8");
            const QColor line("#8a8f99");
            painter.setPen(QPen(page, 1.2));
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(QRectF(12, 2, 24, 24), 2, 2);

            for (int i = 0; i < 5; ++i)
            {
                const qreal y = 7 + i * 4;
                const bool read = kind == UBOptionIcon::ReadFromStart
                                  || (kind == UBOptionIcon::ReadSelection && (i == 1 || i == 2))
                                  || (kind == UBOptionIcon::ReadFromClick && i >= 2);
                const qreal left = (kind == UBOptionIcon::ReadFromClick && i == 2) ? 24 : 16;
                painter.setPen(QPen(read ? blue : line, 2, Qt::SolidLine, Qt::RoundCap));
                painter.drawLine(QPointF(left, y), QPointF(32, y));

                if (kind == UBOptionIcon::ReadFromClick && i == 2)
                {
                    painter.setPen(QPen(line, 2, Qt::SolidLine, Qt::RoundCap));
                    painter.drawLine(QPointF(16, y), QPointF(21, y));
                }
            }

            if (kind == UBOptionIcon::ReadFromStart)
            {
                // An arrow at the first line.
                painter.setPen(Qt::NoPen);
                painter.setBrush(blue);
                painter.drawPolygon(QPolygonF({ QPointF(4, 3.5), QPointF(10, 7), QPointF(4, 10.5) }));
            }
            else if (kind == UBOptionIcon::ReadSelection)
            {
                painter.setPen(QPen(QColor("#ffc400"), 1.2, Qt::DashLine));
                painter.setBrush(QColor(255, 214, 0, 50));
                painter.drawRect(QRectF(14, 8.5, 20, 10));
            }
            else
            {
                // The pointer clicking on the middle line.
                const QPointF tip(23, 14);
                painter.setPen(QPen(QColor("#1c1d20"), 0.8));
                painter.setBrush(Qt::white);
                painter.drawPolygon(QPolygonF({ tip, tip + QPointF(0, 12), tip + QPointF(3, 9), tip + QPointF(5.5, 13.5),
                                                tip + QPointF(7, 12.8), tip + QPointF(4.8, 8.5), tip + QPointF(8.5, 8.5) }));
            }
            break;
        }
        case UBOptionIcon::Play:
        case UBOptionIcon::Pause:
        case UBOptionIcon::Stop:
        case UBOptionIcon::NextPage:
        {
            // The usual signs of players.
            const QColor sign = UBTheme::color("#e8e8e8");
            painter.setPen(Qt::NoPen);
            painter.setBrush(sign);

            if (kind == UBOptionIcon::Play)
            {
                painter.drawPolygon(QPolygonF({ QPointF(18, 4), QPointF(33, 14), QPointF(18, 24) }));
            }
            else if (kind == UBOptionIcon::Pause)
            {
                painter.drawRoundedRect(QRectF(16, 4, 5.5, 20), 1, 1);
                painter.drawRoundedRect(QRectF(26.5, 4, 5.5, 20), 1, 1);
            }
            else if (kind == UBOptionIcon::Stop)
            {
                painter.drawRoundedRect(QRectF(15, 5, 18, 18), 2, 2);
            }
            else
            {
                painter.drawPolygon(QPolygonF({ QPointF(13, 5), QPointF(26, 14), QPointF(13, 23) }));
                painter.drawRoundedRect(QRectF(28, 5, 4.5, 18), 1, 1);
            }
            break;
        }
        case UBOptionIcon::Checklist:
        case UBOptionIcon::Bullets:
        case UBOptionIcon::Numbers:
        {
            // Three lines with a box, a bullet or a number before each.
            const QColor sign = UBTheme::color("#e8e8e8");

            for (int i = 0; i < 3; ++i)
            {
                const qreal y = 6 + i * 8;
                painter.setPen(QPen(sign, 2, Qt::SolidLine, Qt::RoundCap));
                painter.drawLine(QPointF(21, y), QPointF(38, y));

                if (kind == UBOptionIcon::Checklist)
                {
                    painter.setPen(QPen(sign, 1.3));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRect(QRectF(11, y - 3, 6, 6));

                    if (i == 0)
                    {
                        painter.setPen(QPen(blue, 1.6, Qt::SolidLine, Qt::RoundCap));
                        painter.drawPolyline(QPolygonF({ QPointF(12, y), QPointF(13.8, y + 1.8), QPointF(17.5, y - 3) }));
                    }
                }
                else if (kind == UBOptionIcon::Bullets)
                {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(sign);
                    painter.drawEllipse(QPointF(14, y), 2, 2);
                }
                else
                {
                    QFont font = painter.font();
                    font.setPixelSize(7);
                    font.setBold(true);
                    painter.setFont(font);
                    painter.setPen(sign);
                    painter.drawText(QRectF(9, y - 5, 9, 10), Qt::AlignCenter, QString::number(i + 1));
                }
            }
            break;
        }
        case UBOptionIcon::Table:
        {
            // A grid of three rows and three columns, its first row grey.
            const QColor sign = UBTheme::color("#e8e8e8");
            const QRectF grid(13, 4, 22, 20);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(232, 232, 232, 90));
            painter.drawRect(QRectF(grid.left(), grid.top(), grid.width(), grid.height() / 3));
            painter.setPen(QPen(sign, 1.4));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(grid);

            for (int i = 1; i < 3; ++i)
            {
                painter.drawLine(QPointF(grid.left(), grid.top() + grid.height() * i / 3),
                                 QPointF(grid.right(), grid.top() + grid.height() * i / 3));
                painter.drawLine(QPointF(grid.left() + grid.width() * i / 3, grid.top()),
                                 QPointF(grid.left() + grid.width() * i / 3, grid.bottom()));
            }
            break;
        }
        case UBOptionIcon::SelectArrow:
        case UBOptionIcon::SelectLasso:
        {
            // What is selected, in a dashed line, and the pointer that selects.
            painter.setPen(QPen(UBTheme::color("#e8e8e8"), 1.5, Qt::DashLine));
            painter.setBrush(QColor(91, 141, 239, 60));

            if (kind == UBOptionIcon::SelectArrow)
            {
                painter.drawRect(QRectF(6, 3, 28, 18));
            }
            else
            {
                QPainterPath loop(QPointF(8, 12));
                loop.cubicTo(QPointF(6, 0), QPointF(30, -2), QPointF(34, 9));
                loop.cubicTo(QPointF(38, 20), QPointF(20, 26), QPointF(12, 21));
                loop.cubicTo(QPointF(6, 18), QPointF(9, 14), QPointF(8, 12));
                painter.drawPath(loop);
            }

            const QPointF tip(30, 13);
            painter.setPen(QPen(QColor("#1c1d20"), 0.8));
            painter.setBrush(Qt::white);
            painter.drawPolygon(QPolygonF({ tip, tip + QPointF(0, 13), tip + QPointF(3.2, 9.8), tip + QPointF(5.8, 14.6),
                                            tip + QPointF(7.4, 13.8), tip + QPointF(5, 9.2), tip + QPointF(9, 9.2) }));
            break;
        }
        case UBOptionIcon::AiSummary:
        {
            // A page of many lines, and beside it its few main lines.
            const QColor sign = UBTheme::color("#e8e8e8");
            painter.setPen(QPen(sign, 1.4));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRectF(7, 3, 15, 22));

            for (int i = 0; i < 6; ++i)
            {
                painter.drawLine(QPointF(10, 7 + i * 3), QPointF(19, 7 + i * 3));
            }

            painter.setPen(QPen(QColor("#a77bff"), 2.4, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(26, 12), QPointF(29, 12));
            painter.drawLine(QPointF(27, 10), QPointF(29, 12));
            painter.drawLine(QPointF(27, 14), QPointF(29, 12));
            painter.setPen(QPen(sign, 2.2, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(33, 9), QPointF(42, 9));
            painter.drawLine(QPointF(33, 15), QPointF(40, 15));
            drawSparkle(painter, QPointF(40, 22), 4.5, QColor("#ffd36b"));
            break;
        }
        case UBOptionIcon::AiTranslate:
        {
            // A letter of one alphabet becoming a sign of another.
            QFont font = painter.font();
            font.setBold(true);
            font.setPixelSize(15);
            painter.setFont(font);
            painter.setPen(UBTheme::color("#e8e8e8"));
            painter.drawText(QRectF(4, 3, 16, 20), Qt::AlignCenter, "A");
            painter.setPen(QColor("#7ab8ff"));
            painter.drawText(QRectF(28, 7, 17, 20), Qt::AlignCenter, QString::fromUtf8("文"));
            painter.setPen(QPen(QColor("#a77bff"), 2, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(19, 14), QPointF(28, 14));
            painter.drawLine(QPointF(25, 11), QPointF(28, 14));
            painter.drawLine(QPointF(25, 17), QPointF(28, 14));
            drawSparkle(painter, QPointF(23, 5), 3.5, QColor("#ffd36b"));
            break;
        }
        case UBOptionIcon::AiCreateImage:
        {
            // A picture with mountains and a sun, being made.
            const QRectF frame(10, 4, 26, 20);
            painter.setPen(QPen(UBTheme::color("#e8e8e8"), 1.4));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(frame);
            QPainterPath mountains;
            mountains.moveTo(frame.left(), frame.bottom());
            mountains.lineTo(frame.left() + 8, frame.top() + 9);
            mountains.lineTo(frame.left() + 13, frame.top() + 14);
            mountains.lineTo(frame.left() + 18, frame.top() + 8);
            mountains.lineTo(frame.right(), frame.bottom());
            mountains.closeSubpath();
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#5b8def"));
            painter.drawPath(mountains);
            painter.setBrush(QColor("#ffd36b"));
            painter.drawEllipse(QPointF(frame.left() + 7, frame.top() + 5), 2.5, 2.5);
            drawSparkle(painter, QPointF(39, 6), 5, QColor("#a77bff"));
            break;
        }
        case UBOptionIcon::AiRemoveBackground:
        {
            // A person kept on the squares of a transparent background.
            const QRectF frame(12, 3, 24, 22);

            for (int row = 0; row < 4; ++row)
            {
                for (int column = 0; column < 4; ++column)
                {
                    painter.fillRect(QRectF(frame.left() + column * 6, frame.top() + row * 5.5, 6, 5.5),
                                     (row + column) % 2 ? QColor(232, 232, 232, 70) : QColor(232, 232, 232, 160));
                }
            }

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#ff5468"));
            painter.drawEllipse(QPointF(24, 10), 3.5, 3.5);
            painter.drawRoundedRect(QRectF(18, 15, 12, 10), 5, 5);
            drawSparkle(painter, QPointF(40, 6), 4.5, QColor("#ffd36b"));
            break;
        }
        case UBOptionIcon::AiErase:
        {
            // An eraser that leaves the place as if nothing had been there.
            painter.setPen(QPen(UBTheme::color("#e8e8e8"), 1.2, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRectF(6, 12, 14, 12));
            painter.save();
            painter.translate(28, 13);
            painter.rotate(-35);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#ff8fb1"));
            painter.drawRoundedRect(QRectF(-6, -5, 8, 10), 1.5, 1.5);
            painter.setBrush(QColor("#5b8def"));
            painter.drawRoundedRect(QRectF(2, -5, 9, 10), 1.5, 1.5);
            painter.restore();
            drawSparkle(painter, QPointF(40, 20), 5, QColor("#ffd36b"));
            break;
        }
        case UBOptionIcon::AiTranscribe:
        {
            // The waves of a sound becoming lines of text.
            painter.setPen(QPen(QColor("#7ab8ff"), 2, Qt::SolidLine, Qt::RoundCap));
            const qreal heights[] = { 4, 10, 16, 8, 13, 5 };

            for (int i = 0; i < 6; ++i)
            {
                painter.drawLine(QPointF(5 + i * 3, 14 - heights[i] / 2), QPointF(5 + i * 3, 14 + heights[i] / 2));
            }

            painter.setPen(QPen(QColor("#a77bff"), 2, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(24, 14), QPointF(29, 14));
            painter.drawLine(QPointF(27, 12), QPointF(29, 14));
            painter.drawLine(QPointF(27, 16), QPointF(29, 14));
            painter.setPen(QPen(UBTheme::color("#e8e8e8"), 1.8, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(QPointF(33, 8), QPointF(44, 8));
            painter.drawLine(QPointF(33, 13), QPointF(44, 13));
            painter.drawLine(QPointF(33, 18), QPointF(40, 18));
            break;
        }
        case UBOptionIcon::Microphone:
        {
            // A red microphone on its stand.
            const QColor mic("#ff5468");
            painter.setPen(Qt::NoPen);
            painter.setBrush(mic);
            painter.drawRoundedRect(QRectF(20, 2, 8, 15), 4, 4);
            painter.setPen(QPen(UBTheme::color("#e8e8e8"), 1.8, Qt::SolidLine, Qt::RoundCap));
            painter.setBrush(Qt::NoBrush);
            painter.drawArc(QRectF(16, 5, 16, 15), 180 * 16, 180 * 16);
            painter.drawLine(QPointF(24, 20), QPointF(24, 24));
            painter.drawLine(QPointF(19.5, 25), QPointF(28.5, 25));
            break;
        }
    }

    return QIcon(pixmap);
}

static void drawEnding(QPainter& painter, int ending, const QPointF& tip, qreal direction)
{
    // direction: angle in radians the line comes from, the ending points away from it
    const qreal size = 6;
    const QPointF along(std::cos(direction), std::sin(direction));
    const QPointF across(-along.y(), along.x());

    switch (ending)
    {
        case UBLineEnding::OpenArrow:
        {
            QPainterPath path;
            path.moveTo(tip - along * size + across * size * 0.7);
            path.lineTo(tip);
            path.lineTo(tip - along * size - across * size * 0.7);
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
            break;
        }
        case UBLineEnding::ClosedArrow:
        {
            QPolygonF triangle;
            triangle << tip << tip - along * size + across * size * 0.7 << tip - along * size - across * size * 0.7;
            painter.setBrush(painter.pen().color());
            painter.drawPolygon(triangle);
            break;
        }
        case UBLineEnding::Circle:
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(tip - along * size * 0.5, size * 0.5, size * 0.5);
            break;
        case UBLineEnding::Square:
        {
            const QPointF center = tip - along * size * 0.5;
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRectF(center.x() - size * 0.45, center.y() - size * 0.45, size * 0.9, size * 0.9));
            break;
        }
        case UBLineEnding::Diamond:
        {
            const QPointF center = tip - along * size * 0.5;
            QPolygonF diamond;
            diamond << center + along * size * 0.6 << center + across * size * 0.6
                    << center - along * size * 0.6 << center - across * size * 0.6;
            painter.setBrush(Qt::NoBrush);
            painter.drawPolygon(diamond);
            break;
        }
        default:
            break;
    }
}

static QIcon pictureIcon(UBPictureChoice::Kind kind, int index)
{
    const QSize size(40, 24);
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(UBTheme::color("#e8e8e8"), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    switch (kind)
    {
        case UBPictureChoice::Kind::Dash:
        {
            const QList<Qt::PenStyle> styles = { Qt::SolidLine, Qt::DotLine, Qt::DashLine, Qt::DashLine, Qt::DashDotLine };
            pen.setStyle(styles.value(index, Qt::SolidLine));
            pen.setCapStyle(Qt::FlatCap);

            if (index == UBLineDash::LongDash)
            {
                pen.setDashPattern({ 5, 2 });
            }

            painter.setPen(pen);
            painter.drawLine(QPointF(4, 12), QPointF(36, 12));
            break;
        }
        case UBPictureChoice::Kind::BorderStyle:
        {
            painter.setPen(pen);

            if (index == UBBorderStyle::Cloud)
            {
                QPainterPath path;
                path.moveTo(4, 14);

                for (int x = 4; x < 36; x += 8)
                {
                    path.quadTo(x + 4, 4, x + 8, 14);
                }

                painter.drawPath(path);
            }
            else
            {
                painter.drawLine(QPointF(4, 12), QPointF(36, 12));
            }
            break;
        }
        case UBPictureChoice::Kind::StartEnding:
        case UBPictureChoice::Kind::EndEnding:
        {
            painter.setPen(pen);
            const bool atStart = (kind == UBPictureChoice::Kind::StartEnding);
            const QPointF from = atStart ? QPointF(36, 12) : QPointF(4, 12);
            const QPointF tip = atStart ? QPointF(5, 12) : QPointF(35, 12);
            painter.drawLine(from, tip);
            drawEnding(painter, index, tip, atStart ? qDegreesToRadians(180.0) : 0.0);
            break;
        }
    }

    return QIcon(pixmap);
}

UBPictureChoice::UBPictureChoice(Kind kind, int count, QWidget* parent)
    : QWidget(parent)
{
    mGroup = new QButtonGroup(this);
    mGroup->setExclusive(true);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    for (int i = 0; i < count; ++i)
    {
        auto button = new QToolButton(this);
        button->setIcon(pictureIcon(kind, i));
        button->setIconSize(QSize(40, 24));
        button->setCheckable(true);
        mGroup->addButton(button, i);
        layout->addWidget(button);
    }

    layout->addStretch();

    connect(mGroup, &QButtonGroup::idClicked, this, &UBPictureChoice::choiceChanged);
}

void UBPictureChoice::setChoice(int index)
{
    if (auto button = mGroup->button(index))
    {
        button->setChecked(true);

        if (mDropDown)
        {
            mDropDown->setIcon(button->icon());
        }
    }
}

void UBPictureChoice::setDropDown()
{
    if (mDropDown)
    {
        return;
    }

    mDropDown = new QToolButton(this);
    mDropDown->setIconSize(QSize(40, 24));
    mDropDown->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mDropDown->setArrowType(Qt::NoArrow);

    // The other pictures, one under the other, in a small dark window.
    auto popup = new QFrame(this, Qt::Popup);
    popup->setStyleSheet("QFrame { background: #2c313a; border: 1px solid #4a5160; }"
                         "QToolButton { background: transparent; border: none; border-radius: 4px; padding: 3px; }"
                         "QToolButton:hover { background: #3d4452; }"
                         "QToolButton:checked { background: #4a5160; }");
    auto column = new QVBoxLayout(popup);
    column->setContentsMargins(3, 3, 3, 3);
    column->setSpacing(1);

    for (QAbstractButton* button : mGroup->buttons())
    {
        const int id = mGroup->id(button);
        auto item = new QToolButton(popup);
        item->setIcon(button->icon());
        item->setIconSize(QSize(40, 24));
        item->setFixedSize(52, 32);
        item->setToolTip(button->toolTip());
        column->addWidget(item);
        connect(item, &QToolButton::clicked, this, [this, popup, id]() {
            popup->hide();
            setChoice(id);
            emit choiceChanged(id);
        });
        button->hide();
    }

    connect(mDropDown, &QToolButton::clicked, this, [this, popup]() {
        popup->adjustSize();
        popup->move(mDropDown->mapToGlobal(QPoint(0, mDropDown->height())));
        popup->show();
    });

    layout()->addWidget(mDropDown);

    if (QAbstractButton* checked = mGroup->checkedButton())
    {
        mDropDown->setIcon(checked->icon());
    }
}

int UBPictureChoice::choice() const
{
    return mGroup->checkedId();
}

UBStrokePreview::UBStrokePreview(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(56);
}

void UBStrokePreview::setStroke(const QColor& color, qreal width, qreal opacity, bool glow)
{
    mColor = color;
    mWidth = width;
    mOpacity = opacity;
    mGlow = glow;
    update();
}

void UBStrokePreview::setStrokeType(int type)
{
    mStrokeType = type;
    update();
}

void UBStrokePreview::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // A light card, so that dark colors show on the dark panel.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#e9ebef"));
    painter.drawRoundedRect(QRectF(rect()).adjusted(width() * 0.2, 2, -width() * 0.2, -2), 8, 8);

    const qreal left = width() * 0.3;
    const qreal right = width() * 0.7;
    const qreal middle = height() / 2.0;
    const qreal amplitude = height() * 0.22;

    QPainterPath wave;
    wave.moveTo(left, middle);
    wave.cubicTo(left + (right - left) * 0.3, middle - 2 * amplitude,
                 left + (right - left) * 0.7, middle + 2 * amplitude,
                 right, middle);

    const qreal penWidth = qBound(1.0, mWidth, 20.0);

    QColor color = mColor.isValid() ? mColor : QColor(Qt::white);

    if (mGlow)
    {
        QColor glow = color;
        glow.setAlphaF(0.25 * mOpacity);
        painter.setPen(QPen(glow, penWidth * 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(wave);
    }

    color.setAlphaF(mOpacity);

    if (mStrokeType == UBPenStrokeType::Pencil)
    {
        color.setAlphaF(mOpacity * 0.75);
    }

    if (mStrokeType == UBPenStrokeType::Basic)
    {
        painter.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(wave);
    }
    else
    {
        // At least a few pixels wide, so that the change of width shows.
        drawStroke(painter, wave, qMax(penWidth, 4.0), mStrokeType, color);
    }
}

QIcon optionIcon(const QString& picture)
{
    // The picture as tall as the drawings of the other choices, in their width.
    const QSize size(48, 28);
    const qreal ratio = 2;
    QPixmap pixmap(size * ratio);
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    // The pictures of the tools have wide margins: drawn larger than the tile.
    QIcon(picture).paint(&painter, QRect(3, -8, 42, 44));
    painter.end();

    return QIcon(pixmap);
}
