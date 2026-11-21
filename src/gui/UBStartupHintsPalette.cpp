/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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


#include "UBStartupHintsPalette.h"

#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QRegularExpression>
#include <QToolButton>
#include <QVBoxLayout>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "domain/UBWebEngineView.h"
#include "frameworks/UBPlatformUtils.h"

UBStartupHintsPalette::UBStartupHintsPalette(QWidget *parent) :
    UBFloatingPalette(Qt::BottomRightCorner,parent)
{
    setObjectName("UBStartupHintsPalette");
    // Large enough for the pictures of the guide, with the index beside them.
    setFixedSize(1120, 760);

    setStyleSheet(
        "QWidget#hintsHeader { background: #20242b; border-top-left-radius: 10px; border-top-right-radius: 10px; }"
        "QLabel#hintsTitle { color: #f0f0f0; font-size: 16px; font-weight: bold; }"
        "QToolButton#hintsClose { color: #f0f0f0; background: transparent; border: none; font-size: 18px; padding: 0 8px; }"
        "QToolButton#hintsClose:hover { background: #c4314b; border-radius: 4px; }"
        "QLabel#indexTitle { color: #1d3557; font-size: 15px; font-weight: bold; padding: 4px 2px; }"
        "QListWidget#hintsIndex { background: #f6f7f9; border: none; border-left: 1px solid #d6d9de; font-size: 14px; outline: none; }"
        "QListWidget#hintsIndex::item { padding: 6px 8px; color: #222222; border-radius: 4px; }"
        "QListWidget#hintsIndex::item:hover { background: #e6edf7; }"
        "QListWidget#hintsIndex::item:selected { background: #3b5fa0; color: #ffffff; }"
        "QCheckBox { color: #333333; }");

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 8);
    layout->setSpacing(0);

    // A title bar of its own, with the cross that closes; the window is dragged by it.
    auto header = new QWidget(this);
    header->setObjectName("hintsHeader");
    header->setFixedHeight(40);
    auto headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(14, 0, 6, 0);
    auto title = new QLabel(tr("Tips and tricks"), header);
    title->setObjectName("hintsTitle");
    auto close = new QToolButton(header);
    close->setObjectName("hintsClose");
    close->setText(QStringLiteral("✕"));
    close->setToolTip(tr("Close"));
    connect(close, &QToolButton::clicked, this, &QWidget::hide);
    headerLayout->addWidget(title);
    headerLayout->addStretch(1);
    headerLayout->addWidget(close);
    layout->addWidget(header);

    // The page at the left, the index of the pages at the right.
    auto body = new QHBoxLayout();
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);

    mpWebView = new UBWebEngineView(this);
    mpWebView->setAcceptDrops(false);
    body->addWidget(mpWebView, 1);

    auto indexPart = new QWidget(this);
    indexPart->setFixedWidth(270);
    indexPart->setStyleSheet("background: #f6f7f9;");
    auto indexLayout = new QVBoxLayout(indexPart);
    indexLayout->setContentsMargins(8, 8, 8, 4);
    auto indexTitle = new QLabel(tr("Index"), indexPart);
    indexTitle->setObjectName("indexTitle");
    indexLayout->addWidget(indexTitle);
    mIndex = new QListWidget(indexPart);
    mIndex->setObjectName("hintsIndex");
    mIndex->setWordWrap(true);
    mIndex->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    indexLayout->addWidget(mIndex, 1);
    body->addWidget(indexPart);
    layout->addLayout(body, 1);

    auto footer = new QHBoxLayout();
    footer->setContentsMargins(14, 6, 14, 0);
    mShowNextTime = new QCheckBox(tr("Visible next time"), this);
    mShowNextTime->setChecked(UBSettings::settings()->appStartupHintsEnabled->get().toBool());
    connect(mShowNextTime, SIGNAL(stateChanged(int)), this, SLOT(onShowNextTimeStateChanged(int)));
    footer->addStretch(1);
    footer->addWidget(mShowNextTime);
    layout->addLayout(footer);

    fillIndex();
    connect(mIndex, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < mPages.size())
        {
            mpWebView->setUrl(QUrl::fromLocalFile(mPages.at(row)));
        }
    });
    mIndex->setCurrentRow(0);

    hide();
}

UBStartupHintsPalette::~UBStartupHintsPalette()
{
}

void UBStartupHintsPalette::fillIndex()
{
    // The pages in the language of Master Board, else in English.
    QString language = UBSettings::settings()->appPreferredLanguage->get().toString();

    if (language.isEmpty())
    {
        language = UBPlatformUtils::systemLanguage();
    }

    const QString locales = UBSettings::settings()->applicationStartupHintsDirectory() + "/locales/";
    QString folder = locales + language.left(2);

    if (!QFile::exists(folder + "/1.html"))
    {
        folder = locales + "en";
    }

    static const QRegularExpression titleTag("<title>(.*?)</title>", QRegularExpression::DotMatchesEverythingOption);

    for (int number = 1; QFile::exists(QString("%1/%2.html").arg(folder).arg(number)); ++number)
    {
        const QString path = QString("%1/%2.html").arg(folder).arg(number);
        QFile file(path);
        QString name = QString::number(number);

        if (file.open(QIODevice::ReadOnly))
        {
            const QRegularExpressionMatch match = titleTag.match(QString::fromUtf8(file.readAll()));

            if (match.hasMatch())
            {
                name = match.captured(1).trimmed();
            }
        }

        mPages << path;
        new QListWidgetItem(QString("%1. %2").arg(number).arg(name), mIndex);
    }
}

void UBStartupHintsPalette::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // A white card with a thin edge, the title bar painted by its own widget.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#9aa1ab"), 1));
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 10, 10);
}

void UBStartupHintsPalette::onShowNextTimeStateChanged(int state)
{
    UBSettings::settings()->appStartupHintsEnabled->setBool(state == Qt::Checked);
}

void UBStartupHintsPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    adjustSizeAndPosition();
    move((parentWidget()->width() - width()) / 2, qMax(0, (parentWidget()->height() - height()) / 5));
}

int UBStartupHintsPalette::border()
{
    return 0;
}
