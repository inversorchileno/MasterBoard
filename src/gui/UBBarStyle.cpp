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



#include "UBBarStyle.h"

#include <QApplication>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>

#include "core/UBSettings.h"
#include "gui/UBFloatingPalette.h"

void UBBarStyle::paint(QPainter& painter, const QRectF& bar, int edge)
{
    UBSettings* settings = UBSettings::settings();
    const qreal opacity = qBound(10, settings->barOpacity->get().toInt(), 100) / 100.0;

    QColor inner(settings->barInnerColor->get().toString());
    QColor outer(settings->barOuterColor->get().toString());
    inner.setAlphaF(opacity);
    outer.setAlphaF(opacity);

    // From square corners at 0 to a round end at 100.
    const qreal roundness = qBound(0, settings->barRoundness->get().toInt(), 100) / 100.0;
    const qreal radius = roundness * qMin(bar.width(), bar.height()) / 2;
    const QRectF inside = bar.adjusted(edge, edge, -edge, -edge);
    const qreal insideRadius = qMax<qreal>(0, radius - edge);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // The edge as a ring, so that the inside is not painted over it.
    QPainterPath ring;
    ring.addRoundedRect(bar, radius, radius);
    ring.addRoundedRect(inside, insideRadius, insideRadius);
    painter.setBrush(outer);
    painter.drawPath(ring);

    painter.setBrush(inner);
    painter.drawRoundedRect(inside, insideRadius, insideRadius);
    painter.restore();
}

void UBBarStyle::repaintPalettes()
{
    for (QWidget* widget : QApplication::allWidgets())
    {
        if (qobject_cast<UBFloatingPalette*>(widget))
        {
            widget->update();
        }
    }
}

void UBBarStyle::openWindow(QWidget* parent)
{
    UBSettings* settings = UBSettings::settings();

    // Given back by Cancel.
    const QVariant innerBefore = settings->barInnerColor->get();
    const QVariant outerBefore = settings->barOuterColor->get();
    const QVariant opacityBefore = settings->barOpacity->get();
    const QVariant roundnessBefore = settings->barRoundness->get();

    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Bar style"));
    auto layout = new QVBoxLayout(&dialog);

    auto help = new QLabel(QObject::tr("The style of the tool palettes, on the board and on the desktop. "
                                       "The palettes change at once."), &dialog);
    help->setWordWrap(true);
    layout->addWidget(help);

    auto grid = new QGridLayout();
    layout->addLayout(grid);

    auto colorButton = [&dialog](UBSetting* setting) {
        auto button = new QToolButton(&dialog);
        button->setFixedSize(64, 26);

        auto show = [button](const QColor& color) {
            button->setStyleSheet(QString("QToolButton { background: %1; border: 1px solid #555555; border-radius: 4px; }").arg(color.name()));
        };
        show(QColor(setting->get().toString()));

        QObject::connect(button, &QToolButton::clicked, &dialog, [&dialog, setting, show]() {
            const QColor color = QColorDialog::getColor(QColor(setting->get().toString()), &dialog);

            if (color.isValid())
            {
                setting->set(color.name());
                show(color);
                repaintPalettes();
            }
        });

        // Restore gives the default back: the button follows.
        QObject::connect(setting, &UBSetting::changed, button, [show](const QVariant& value) { show(QColor(value.toString())); });
        return button;
    };

    grid->addWidget(new QLabel(QObject::tr("Color inside"), &dialog), 0, 0);
    grid->addWidget(colorButton(settings->barInnerColor), 0, 1, Qt::AlignLeft);
    grid->addWidget(new QLabel(QObject::tr("Color of the edge"), &dialog), 1, 0);
    grid->addWidget(colorButton(settings->barOuterColor), 1, 1, Qt::AlignLeft);

    auto slider = [&dialog](UBSetting* setting, int minimum, const QString& left, const QString& right) {
        auto row = new QWidget(&dialog);
        auto rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto value = new QSlider(Qt::Horizontal, row);
        value->setRange(minimum, 100);
        value->setValue(setting->get().toInt());
        value->setMinimumWidth(240);
        auto number = new QLabel(QString("%1 %").arg(value->value()), row);
        number->setMinimumWidth(44);

        if (!left.isEmpty())
        {
            rowLayout->addWidget(new QLabel(left, row));
        }

        rowLayout->addWidget(value, 1);

        if (!right.isEmpty())
        {
            rowLayout->addWidget(new QLabel(right, row));
        }

        rowLayout->addWidget(number);

        QObject::connect(value, &QSlider::valueChanged, row, [setting, number](int chosen) {
            setting->set(chosen);
            number->setText(QString("%1 %").arg(chosen));
            repaintPalettes();
        });
        QObject::connect(setting, &UBSetting::changed, value, [value](const QVariant& chosen) {
            const QSignalBlocker blocker(value);
            value->setValue(chosen.toInt());
        });
        return row;
    };

    grid->addWidget(new QLabel(QObject::tr("Opacity"), &dialog), 2, 0);
    grid->addWidget(slider(settings->barOpacity, 10, QString(), QString()), 2, 1);
    grid->addWidget(new QLabel(QObject::tr("Corners"), &dialog), 3, 0);
    grid->addWidget(slider(settings->barRoundness, 0, QObject::tr("Square"), QObject::tr("Round")), 3, 1);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults, &dialog);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, &dialog, [settings]() {
        settings->barInnerColor->reset();
        settings->barOuterColor->reset();
        settings->barOpacity->reset();
        settings->barRoundness->reset();
        repaintPalettes();
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        settings->save();
        return;
    }

    settings->barInnerColor->set(innerBefore);
    settings->barOuterColor->set(outerBefore);
    settings->barOpacity->set(opacityBefore);
    settings->barRoundness->set(roundnessBefore);
    repaintPalettes();
}
