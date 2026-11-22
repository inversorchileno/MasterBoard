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




#include "UBPreferencesController.h"
#include "gui/UBBarStyle.h"

#include "core/UBGitHubUpdater.h"
#include "frameworks/UBPlatformUtils.h"

#include <QButtonGroup>
#include <QMessageBox>
#include <QTimer>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QListWidget>
#include <QScrollArea>
#include <QStackedWidget>
#include <QCheckBox>
#include <QColorDialog>
#include <QSpinBox>
#include <QToolButton>
#include <QComboBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QTabWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "UBSettings.h"
#include "UBApplication.h"

#include "gui/UBCircleFrame.h"

#include "core/UBSetting.h"
#include "gui/UBMainWindow.h"
#include "gui/UBTheme.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "frameworks/UBStringUtils.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "gui/UBStylusPalette.h"
#include "gui/UBAiPanel.h"
#include "desktop/UBDesktopFrame.h"
#include "document/UBDocumentController.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBDrawingController.h"
#include "podcast/UBPodcastController.h"

#include "ui_preferences.h"

#include "core/memcheck.h"


qreal UBPreferencesController::sSliderRatio = 10.0;
qreal UBPreferencesController::sMinPenWidth = 0.5;
qreal UBPreferencesController::sMaxPenWidth = 50.0;


UBPreferencesDialog::UBPreferencesDialog(UBPreferencesController* prefController, QWidget* parent,Qt::WindowFlags f)
    :QDialog(parent,f)
    ,mPreferencesController(prefController)
{
}

UBPreferencesDialog::~UBPreferencesDialog()
{
}

void UBPreferencesDialog::closeEvent(QCloseEvent* e)
{
    // As Cancel: the settings changed since the window opened, or since Apply, are given back.
    e->ignore();
    reject();
}

void UBPreferencesDialog::keyPressEvent(QKeyEvent* e)
{
    // Enter in a field does not close the window.
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
    {
        e->accept();
        return;
    }

    QDialog::keyPressEvent(e);
}



UBPreferencesController::UBPreferencesController(QWidget *parent)
    : QObject(parent)
    , mPreferencesWindow(0)
    , mPreferencesUI(0)
    , mPenProperties(0)
    , mMarkerProperties(0)
    , mDarkBackgroundGridColorPicker(0)
    , mLightBackgroundGridColorPicker(0)
{
    mPreferencesWindow = new UBPreferencesDialog(this,parent, Qt::Dialog);
    mPreferencesUI = new Ui::preferencesDialog();  // deleted in destructor
    mPreferencesUI->setupUi(mPreferencesWindow);
    addLanguageChoice();
    adjustScreensPreferences();

    // Light background and black text, readable whatever the theme of the system.
    mPreferencesWindow->setStyleSheet(
        "QDialog, QTabWidget::pane, QScrollArea, QScrollArea > QWidget > QWidget { background: #f1f1f1; }"
        "QLabel, QCheckBox, QRadioButton, QGroupBox { color: #111111; }"
        "QGroupBox { font-weight: bold; }"
        "QLabel:disabled, QCheckBox:disabled, QRadioButton:disabled, QGroupBox:disabled { color: #8a8a8a; }");

    connect(UBApplication::displayManager, &UBDisplayManager::availableScreenCountChanged, this, &UBPreferencesController::adjustScreensPreferences);
    wire();
}


UBPreferencesController::~UBPreferencesController()
{
    delete mPreferencesWindow;

    delete mPreferencesUI;

    delete mPenProperties;

    delete mMarkerProperties;
}

void UBPreferencesController::addLanguageChoice()
{
    // First option of the Display tab, above the other settings.
    auto group = new QGroupBox(tr("Language"), mPreferencesUI->displayTab);
    mLanguageGroup = group;
    auto layout = new QHBoxLayout(group);

    auto languages = new QComboBox(group);
    languages->addItem(tr("Language of the system"), QString());

    QStringList codes = UBPlatformUtils::availableTranslations();
    codes.removeDuplicates();
    QList<QPair<QString, QString>> named;

    for (const QString& code : std::as_const(codes))
    {
        QString name = QLocale(code).nativeLanguageName();

        if (name.isEmpty())
        {
            name = code;
        }

        name[0] = name.at(0).toUpper();
        named << qMakePair(QString("%1 (%2)").arg(name, code), code);
    }

    std::sort(named.begin(), named.end());

    for (const auto& language : std::as_const(named))
    {
        languages->addItem(language.first, language.second);
    }

    const QString current = UBSettings::settings()->appPreferredLanguage->get().toString();
    int currentIndex = 0;

    for (int i = 1; i < languages->count() && !current.isEmpty(); ++i)
    {
        if (languages->itemData(i).toString() == current)
        {
            currentIndex = i;
        }
    }

    languages->setCurrentIndex(currentIndex);
    languages->setMinimumWidth(240);

    auto hint = new QLabel(tr("The new language is used the next time Master Board starts."), group);

    layout->addWidget(languages);
    layout->addWidget(hint, 1);

    connect(languages, qOverload<int>(&QComboBox::activated), this, [languages](int index) {
        UBSettings::settings()->appPreferredLanguage->set(languages->itemData(index).toString());
    });

    // The rest of the Display tab moves down under the language.
    QGridLayout* tabLayout = mPreferencesUI->gridLayout_3;
    tabLayout->removeWidget(mPreferencesUI->scrollArea);
    tabLayout->addWidget(group, 0, 0);
    tabLayout->addWidget(mPreferencesUI->scrollArea, 1, 0);
}

void UBPreferencesController::simplifyTabs()
{
    QTabWidget* tabs = mPreferencesUI->mainTabWidget;
    tabs->setTabText(tabs->indexOf(mPreferencesUI->displayTab), tr("General"));

    // Pressure sensitivity of the pen and the marker, under the language.
    auto pressure = new QGroupBox(tr("Pressure of the pen"), mPreferencesUI->displayTab);
    mPressureGroup = pressure;
    auto pressureLayout = new QVBoxLayout(pressure);
    pressureLayout->addWidget(mPenProperties->pressureSensitiveCheckBox);
    pressureLayout->addWidget(mMarkerProperties->pressureSensitiveCheckBox);

    QGridLayout* displayLayout = mPreferencesUI->gridLayout_3;
    displayLayout->removeWidget(mPreferencesUI->scrollArea);
    displayLayout->addWidget(pressure, 1, 0);
    mBorderGroup = desktopBorderChoice();
    displayLayout->addWidget(mBorderGroup, 2, 0);
    displayLayout->addWidget(mPreferencesUI->scrollArea, 3, 0);

    tabs->removeTab(tabs->indexOf(mPreferencesUI->penTab));
    // The color of the lines of the page is in Template style.
    tabs->removeTab(tabs->indexOf(mPreferencesUI->gridTab));
    tabs->removeTab(tabs->indexOf(mPreferencesUI->markerTab));

    // The licenses in the About tab, behind a small button at its bottom:
    // out of the way, but there.
    QWidget* licenses = mPreferencesUI->thirdPartyLicence;
    tabs->removeTab(tabs->indexOf(licenses));
    tabs->removeTab(tabs->indexOf(mPreferencesUI->creditsTab));

    QVBoxLayout* about = mPreferencesUI->verticalLayout_2;
    auto buttons = new QHBoxLayout();
    buttons->addStretch(1);
    about->addLayout(buttons);

    const QList<QPair<QString, QWidget*>> hidden = {
        { tr("References"), licenses },
    };

    for (const auto& part : hidden)
    {
        QWidget* content = part.second;
        about->addWidget(content, 1);
        content->hide();

        auto button = new QToolButton(mPreferencesUI->aboutTab);
        button->setText(part.first + QStringLiteral(" ▸"));
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setStyleSheet("QToolButton { color: #6b7383; font-size: 11px; border: none; }"
                              "QToolButton:checked { color: #2f5fa8; }");
        buttons->addWidget(button);

        const QString name = part.first;
        connect(button, &QToolButton::toggled, this, [button, content, name](bool shown) {
            content->setVisible(shown);
            button->setText(name + (shown ? QStringLiteral(" ▾") : QStringLiteral(" ▸")));
        });
    }

    mAiPage = aiChoice();
    tabs->insertTab(1, mAiPage, tr("AI"));
}

QWidget* UBPreferencesController::aiChoice()
{
    // The AI offered by the AI panel: a name and the address of its page.
    auto page = new QWidget(mPreferencesUI->mainTabWidget);
    auto layout = new QVBoxLayout(page);

    auto help = new QLabel(tr("The AI that the AI panel can show. Write a name and the address of its page; "
                              "the first one is shown the first time."), page);
    help->setWordWrap(true);
    layout->addWidget(help);

    auto table = new QTableWidget(0, 2, page);
    table->setHorizontalHeaderLabels({ tr("Name"), tr("Address") });
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->verticalHeader()->hide();
    layout->addWidget(table, 1);

    auto fill = [table](const QStringList& services) {
        const QSignalBlocker blocker(table);
        table->setRowCount(0);

        for (const QString& service : services)
        {
            const int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(service.section('|', 0, 0)));
            table->setItem(row, 1, new QTableWidgetItem(service.section('|', 1)));
        }
    };

    auto save = [table]() {
        QStringList services;

        for (int row = 0; row < table->rowCount(); ++row)
        {
            const QString name = table->item(row, 0) ? table->item(row, 0)->text().trimmed() : QString();
            const QString address = table->item(row, 1) ? table->item(row, 1)->text().trimmed() : QString();

            if (!name.isEmpty() && !address.isEmpty())
            {
                services << name + "|" + address;
            }
        }

        UBSettings::settings()->aiServices->set(services);
    };

    fill(UBSettings::settings()->aiServices->get().toStringList());
    connect(table, &QTableWidget::itemChanged, this, save);

    auto buttons = new QHBoxLayout();
    auto add = new QPushButton(tr("Add"), page);
    auto remove = new QPushButton(tr("Remove"), page);
    auto restore = new QPushButton(tr("Default list"), page);
    buttons->addWidget(add);
    buttons->addWidget(remove);
    buttons->addStretch(1);
    buttons->addWidget(restore);
    layout->addLayout(buttons);

    connect(add, &QPushButton::clicked, this, [table]() {
        const int row = table->rowCount();
        table->insertRow(row);
        auto name = new QTableWidgetItem();
        table->setItem(row, 0, name);
        table->setItem(row, 1, new QTableWidgetItem("https://"));
        table->setCurrentItem(name);
        table->editItem(name);
    });
    connect(remove, &QPushButton::clicked, this, [table, save]() {
        if (table->currentRow() >= 0)
        {
            table->removeRow(table->currentRow());
            save();
        }
    });
    connect(restore, &QPushButton::clicked, this, [fill]() {
        UBSettings::settings()->aiServices->set(UBAiPanel::defaultServices());
        fill(UBAiPanel::defaultServices());
    });

    return page;
}

QWidget* UBPreferencesController::themeChoice()
{
    // The colors of the menu bar, of the options of the tools and of the zoom.
    auto group = new QGroupBox(tr("Theme"), mPreferencesWindow);
    auto layout = new QVBoxLayout(group);
    auto choices = new QButtonGroup(group);

#ifdef Q_OS_WIN
    const QString system = tr("Same as Windows");
#else
    const QString system = tr("Same as the system");
#endif

    const QList<QPair<QString, QString>> themes = {
        { "system", system },
        { "dark", tr("Dark") },
        { "white", tr("White") },
        { "grey", tr("Grey") },
    };

    const QString current = UBSettings::settings()->appTheme->get().toString();
    auto row = new QHBoxLayout();

    for (const auto& theme : themes)
    {
        auto choice = new QRadioButton(theme.second, group);
        choice->setProperty("theme", theme.first);
        choice->setChecked(theme.first == current);
        choices->addButton(choice);
        row->addWidget(choice);
    }

    row->addStretch(1);
    layout->addLayout(row);

    auto hint = new QLabel(tr("The colors of the menu bar, of the options of the tools and of the zoom. "
                              "To show a new theme, Master Board restarts by itself."), group);
    hint->setWordWrap(true);
    hint->setObjectName("hint");
    layout->addWidget(hint);

    connect(choices, &QButtonGroup::buttonClicked, this, [](QAbstractButton* button) {
        UBSettings::settings()->appTheme->set(button->property("theme").toString());
    });

    // The choice follows the setting, also when Cancel gives it back.
    connect(UBSettings::settings()->appTheme, &UBSetting::changed, group, [choices](const QVariant& value) {
        for (QAbstractButton* button : choices->buttons())
        {
            button->setChecked(button->property("theme").toString() == value.toString());
        }
    });

    return group;
}

QWidget* UBPreferencesController::categoryPage(const QIcon& icon, const QString& title, const QList<QWidget*>& sections)
{
    // The title of the category with its icon, then its sections, which scroll.
    auto page = new QWidget(mPreferencesWindow);
    auto layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto header = new QWidget(page);
    header->setObjectName("categoryHeader");
    auto headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(10, 6, 10, 6);
    auto iconLabel = new QLabel(header);
    iconLabel->setPixmap(icon.pixmap(28, 28));
    auto titleLabel = new QLabel(title, header);
    titleLabel->setObjectName("categoryTitle");
    headerLayout->addStretch(1);
    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch(1);
    layout->addWidget(header);

    auto scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto content = new QWidget(scroll);
    content->setObjectName("categoryContent");
    auto contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(6, 4, 10, 4);
    contentLayout->setSpacing(10);

    for (QWidget* section : sections)
    {
        if (section)
        {
            section->setParent(content);
            section->show();
            contentLayout->addWidget(section);
        }
    }

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    layout->addWidget(scroll, 1);
    return page;
}

void UBPreferencesController::buildCategories()
{
    Ui::preferencesDialog* ui = mPreferencesUI;

    // The tabs and the version above them give their place to the categories.
    ui->mainTabWidget->hide();
    ui->versionLabel->hide();
    ui->closeButton->hide();

    auto body = new QWidget(mPreferencesWindow);
    auto bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(8);

    auto left = new QFrame(body);
    left->setObjectName("categoryFrame");
    auto leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(4, 4, 4, 4);
    auto leftTitle = new QLabel(tr("Categories"), left);
    leftTitle->setObjectName("categoryTitle");
    leftTitle->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(leftTitle);

    auto categories = new QListWidget(left);
    categories->setObjectName("categories");
    categories->setIconSize(QSize(22, 22));
    categories->setFixedWidth(260);
    categories->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftLayout->addWidget(categories, 1);

    auto right = new QFrame(body);
    right->setObjectName("categoryFrame");
    auto rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(4, 4, 4, 4);
    auto pages = new QStackedWidget(right);
    rightLayout->addWidget(pages);

    bodyLayout->addWidget(left);
    bodyLayout->addWidget(right, 1);

    // The page About: the version, the text, and the references at the bottom.
    auto version = new QLabel(QString("Master Board %1").arg(UBGitHubUpdater::currentVersionText()), ui->aboutTab);
    version->setObjectName("version");
    ui->verticalLayout_2->insertWidget(0, version);
    ui->aboutTab->setObjectName("aboutPage");

    QWidget* network = ui->networkTab;
    network->setObjectName("networkPage");
    ui->internetGroupBox->setTitle(tr("Internet"));

    QWidget* updates = ui->softwareUpdateGroupBox;
    auto updatesHint = new QLabel(tr("Master Board %1. To look for a new version now: About > Update.")
                                      .arg(UBGitHubUpdater::currentVersionText()), updates);
    updatesHint->setObjectName("hint");
    updatesHint->setWordWrap(true);
    auto updatesLayout = new QVBoxLayout(updates);
    updatesLayout->addWidget(ui->checkSoftwareUpdateAtLaunchCheckBox);
    updatesLayout->addWidget(updatesHint);
    updates->setMinimumHeight(0);
    updates->setMaximumHeight(QWIDGETSIZE_MAX);

    // The choices of the toolbar one under the other, to fit the width.
    // "Toolbar" is the name of the palette of the tools; this one is the bar of their options.
    auto toolbar = new QGroupBox(tr("Options bar"), mPreferencesWindow);
    ui->stylusPaletteGroupBox->setTitle(tr("Toolbar"));
    auto toolbarLayout = new QVBoxLayout(toolbar);
    toolbarLayout->addWidget(ui->toolbarAtTopRadioButton);
    toolbarLayout->addWidget(ui->toolbarAtBottomRadioButton);
    toolbarLayout->addWidget(ui->toolbarDisplayTextCheckBox);
    ui->toolbarGroupBox->hide();

    const struct
    {
        QString icon;
        QString name;
        QString title;
        QList<QWidget*> sections;
    } list[] = {
        { ":/images/toolbar/settings.png", tr("General"), tr("General settings"),
          { mLanguageGroup, ui->modeGroupBox, ui->keyboardGroupBox } },
        { ":/images/color.svg", tr("Appearance"), tr("Appearance"),
          { themeChoice(), toolbar } },
        { ":/images/toolbar/stylus.png", tr("Pen and toolbar"), tr("Pen and toolbar"),
          { mPressureGroup, ui->stylusPaletteGroupBox } },
        { ":/images/toolbar/display.png", tr("Screens and desktop"), tr("Screens and desktop"),
          { mBorderGroup, ui->multiDisplayGroupBox } },
        { ":/images/toolbar/pdf.png", tr("Documents and PDF"), tr("Documents and PDF"),
          { ui->documentsGroupBox, ui->PDFGroupBox } },
        { QString(), tr("AI"), tr("Artificial intelligence"), { mAiPage } },
        { ":/images/toolbar/web.png", tr("Network"), tr("Network"), { ui->internetGroupBox } },
        { ":/images/toolbar/updates.png", tr("Update"), tr("Update"), { updates } },
        { ":/images/toolbar/tip.png", tr("About"), tr("About Master Board"), { ui->aboutTab } },
    };

    for (const auto& category : list)
    {
        const QIcon icon = category.icon.isEmpty() ? UBApplication::mainWindow->actionAi->icon() : QIcon(category.icon);
        new QListWidgetItem(icon, category.name, categories);
        pages->addWidget(categoryPage(icon, category.title, category.sections));
    }

    connect(categories, &QListWidget::currentRowChanged, pages, &QStackedWidget::setCurrentIndex);
    categories->setCurrentRow(0);

    auto grid = qobject_cast<QGridLayout*>(mPreferencesWindow->layout());
    grid->addWidget(body, 2, 0);
    grid->setRowStretch(2, 1);

    // Accept, Apply and Cancel at the bottom right.
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel, mPreferencesWindow);
    ui->horizontalLayout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &UBPreferencesController::close);
    connect(buttons, &QDialogButtonBox::rejected, mPreferencesWindow, &QDialog::reject);
    connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &UBPreferencesController::applyChanges);
    connect(mPreferencesWindow, &QDialog::rejected, this, &UBPreferencesController::restoreSnapshot);

    // The default settings are those of the general categories.
    disconnect(ui->defaultSettingsButton, nullptr, this, nullptr);
    connect(ui->defaultSettingsButton, &QPushButton::released, this, [this]() {
        mPreferencesUI->mainTabWidget->setCurrentWidget(mPreferencesUI->displayTab);
        defaultSettings();
    });

    mPreferencesWindow->resize(980, 640);
    mPreferencesWindow->setStyleSheet(
        "QDialog { background: #e9ebee; }"
        "QFrame#categoryFrame { background: #f6f7f9; border: 1px solid #c9ced6; border-radius: 4px; }"
        "QLabel#categoryTitle { color: #1d3557; font-size: 17px; background: transparent; }"
        "QWidget#categoryHeader { background: transparent; }"
        "QListWidget#categories { background: #ffffff; border: 1px solid #d6b85a; font-size: 14px; outline: none; }"
        "QListWidget#categories::item { padding: 4px 2px; color: #111111; }"
        "QListWidget#categories::item:selected { background: #cfe2fb; border: 1px solid #7fa7dd; color: #111111; }"
        "QScrollArea, QWidget#categoryContent, QWidget#aboutPage, QWidget#networkPage { background: #f6f7f9; }"
        "QGroupBox { background: #eceef1; border: 1px solid #d3d7dd; border-radius: 4px; margin-top: 26px;"
        " padding: 10px 8px 8px 8px; color: #111111; font-weight: normal; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 6px; top: 4px;"
        " padding: 0 2px; color: #1d3557; }"
        "QLabel, QCheckBox, QRadioButton { color: #111111; background: transparent; }"
        "QLabel#hint { color: #5b6472; }"
        "QCheckBox::indicator, QRadioButton::indicator { width: 14px; height: 14px; border: 1px solid #7a8496; background: #ffffff; }"
        "QCheckBox::indicator { border-radius: 3px; }"
        "QRadioButton::indicator { border-radius: 8px; }"
        "QCheckBox::indicator:checked { background: #2f5fa8; border-color: #2f5fa8; image: url(:/images/checkmark.svg); }"
        "QRadioButton::indicator:checked { background: qradialgradient(cx: 0.5, cy: 0.5, radius: 0.5, fx: 0.5, fy: 0.5,"
        " stop: 0 #2f5fa8, stop: 0.45 #2f5fa8, stop: 0.55 #ffffff, stop: 1 #ffffff); border-color: #2f5fa8; }"
        "QCheckBox::indicator:disabled, QRadioButton::indicator:disabled { background: #e3e5e8; border-color: #b5bcc7; }"
        "QLabel#version { color: #1d3557; font-size: 15px; font-weight: bold; padding: 4px 0; }"
        "QLabel:disabled, QCheckBox:disabled, QRadioButton:disabled, QGroupBox:disabled { color: #8a8a8a; }");
}

QWidget* UBPreferencesController::desktopBorderChoice()
{
    UBSettings* settings = UBSettings::settings();

    // The border around the screen while the desktop mode is on.
    auto group = new QGroupBox(tr("Border in desktop mode"), mPreferencesUI->displayTab);
    auto layout = new QHBoxLayout(group);

    auto shown = new QCheckBox(tr("Show a colored border around the screen"), group);
    shown->setChecked(settings->desktopBorderEnabled->get().toBool());

    auto colorButton = new QToolButton(group);
    colorButton->setToolTip(tr("Color of the border"));
    colorButton->setFixedSize(40, 22);

    auto showColor = [colorButton](const QColor& color) {
        colorButton->setStyleSheet(QString("QToolButton { background: %1; border: 1px solid #555555; border-radius: 3px; }").arg(color.name()));
    };
    showColor(QColor(settings->desktopBorderColor->get().toString()));

    auto widthLabel = new QLabel(tr("Width"), group);
    auto width = new QSpinBox(group);
    width->setRange(1, UBDesktopFrame::sMaximumWidth);
    width->setSuffix(tr(" px"));
    width->setValue(settings->desktopBorderWidth->get().toInt());

    layout->addWidget(shown);
    layout->addSpacing(16);
    layout->addWidget(colorButton);
    layout->addSpacing(16);
    layout->addWidget(widthLabel);
    layout->addWidget(width);
    layout->addStretch(1);

    auto enable = [colorButton, widthLabel, width](bool on) {
        colorButton->setEnabled(on);
        widthLabel->setEnabled(on);
        width->setEnabled(on);
    };
    enable(shown->isChecked());

    connect(shown, &QCheckBox::toggled, this, [settings, enable](bool on) {
        settings->desktopBorderEnabled->set(on);
        enable(on);
    });
    connect(colorButton, &QToolButton::clicked, this, [this, settings, showColor]() {
        const QColor color = QColorDialog::getColor(QColor(settings->desktopBorderColor->get().toString()),
                                                    mPreferencesWindow, tr("Color of the border"));

        if (color.isValid())
        {
            settings->desktopBorderColor->set(color.name());
            showColor(color);
        }
    });
    connect(width, qOverload<int>(&QSpinBox::valueChanged), this, [settings](int value) {
        settings->desktopBorderWidth->set(value);
    });

    return group;
}

void UBPreferencesController::adjustScreensPreferences()
{
    bool enabled = UBApplication::displayManager->numScreens() > 1;
    mPreferencesUI->multiDisplayGroupBox->setEnabled(enabled);

    if (enabled)
    {
        mPreferencesUI->screenList->setPlaceholderText(tr("Use all available displays"));
    }
    else
    {
        mPreferencesUI->screenList->setPlaceholderText("");
    }

    auto availableScreens = UBApplication::displayManager->availableScreens();
    QStringList screenNames;
    static QRegularExpression specialChars("[^a-zA-Z0-9]+");

    for (QScreen* screen : availableScreens)
    {
        screenNames << screen->name().replace(specialChars, "-");
    }

    QString screenConfiguration = screenNames.join('_');

    QString path = UBSettings::settings()->appScreenList->path() + "-" + screenConfiguration;

    if (path != mScreenConfigurationPath)
    {
        mScreenConfigurationPath = path;
        QVariant value = UBSettings::settings()->value(path);
        UBSettings::settings()->appScreenList->set(value);
        mPreferencesUI->screenList->loadScreenList(value.toStringList());
    }
}

void UBPreferencesController::show()
{
    init();
    takeSnapshot();

    mPreferencesWindow->exec();
}

void UBPreferencesController::takeSnapshot()
{
    mSnapshot.clear();

    for (UBSetting* setting : UBSettings::settings()->findChildren<UBSetting*>())
    {
        mSnapshot.insert(setting, setting->get());
    }
}

void UBPreferencesController::restoreSnapshot()
{
    bool changed = false;

    for (auto it = mSnapshot.cbegin(); it != mSnapshot.cend(); ++it)
    {
        if (it.key()->get() != it.value())
        {
            it.key()->set(it.value());
            changed = true;
        }
    }

    if (changed)
    {
        // What follows its setting only when it is chosen.
        UBApplication::boardController->paletteManager()->placeStylusPalette(UBStylusPalette::edge());
        UBApplication::documentController->refreshDateColumns();
        init();
    }
}

void UBPreferencesController::applyChanges()
{
    // A new theme is seen once Master Board starts again: it restarts.
    const bool newTheme = UBSettings::settings()->appTheme->get().toString() != UBTheme::chosenAtStart();

    UBSettings::settings()->webHomePage->set(mPreferencesUI->webHomePage->text());
    UBSettings::settings()->setProxyUsername(mPreferencesUI->proxyUsername->text());
    UBSettings::settings()->setProxyPassword(mPreferencesUI->proxyPassword->text());
    UBSettings::settings()->save();
    takeSnapshot();

    if (newTheme && !mRestartAsked)
    {
        mRestartAsked = true;
        const auto answer = QMessageBox::question(mPreferencesWindow, tr("Theme"),
            tr("Master Board will restart to show the new theme. Your documents are saved first."),
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

        if (answer == QMessageBox::Ok)
        {
            mPreferencesWindow->accept();
            QTimer::singleShot(0, qApp, []() { UBApplication::restart(); });
        }
    }
}

void UBPreferencesController::wire()
{
    UBSettings* settings = UBSettings::settings();

    // main tab
    mPreferencesUI->mainTabWidget->setCurrentWidget(mPreferencesUI->displayTab);
    mPreferencesUI->versionLabel->setText(QString("Master Board %1").arg(UBGitHubUpdater::currentVersionText()));

    connect(mPreferencesUI->closeButton, SIGNAL(released()), this, SLOT(close()));
    connect(mPreferencesUI->defaultSettingsButton, SIGNAL(released()), this, SLOT(defaultSettings()));

    connect(mPreferencesUI->screenList, &UBScreenListLineEdit::screenListChanged, this, [this,settings](const QStringList screenList){
        settings->appScreenList->set(screenList);

        if (!mScreenConfigurationPath.isEmpty())
        {
            settings->setValue(mScreenConfigurationPath, screenList);
        }
    });

    // OSK preferences

    for(int i = 0; i < settings->supportedKeyboardSizes->size(); i++)
        mPreferencesUI->keyboardPaletteKeyButtonSize->addItem(settings->supportedKeyboardSizes->at(i));


    connect(mPreferencesUI->useSystemOSKCheckBox, SIGNAL(clicked(bool)), settings->useSystemOnScreenKeyboard, SLOT(setBool(bool)));
    connect(mPreferencesUI->useSystemOSKCheckBox, SIGNAL(clicked(bool)), this, SLOT(systemOSKCheckBoxToggled(bool)));

    // PDF preferences
    connect(mPreferencesUI->exportBackgroundGrid, SIGNAL(clicked(bool)), settings->exportBackgroundGrid, SLOT(setBool(bool)));
    connect(mPreferencesUI->exportBackgroundColor, SIGNAL(clicked(bool)), settings->exportBackgroundColor, SLOT(setBool(bool)));

    // Documents Mode preferences
    connect(mPreferencesUI->showDateColumnOnAlphabeticalSort, SIGNAL(clicked(bool)), settings->showDateColumnOnAlphabeticalSort, SLOT(setBool(bool)));
    connect(mPreferencesUI->showDateColumnOnAlphabeticalSort, SIGNAL(clicked(bool)), UBApplication::documentController, SLOT(refreshDateColumns()));
    connect(mPreferencesUI->emptyTrashForOlderDocuments, SIGNAL(clicked(bool)), settings->emptyTrashForOlderDocuments, SLOT(setBool(bool)));
    connect(mPreferencesUI->emptyTrashDaysValue, SIGNAL(valueChanged(int)), settings->emptyTrashDaysValue,  SLOT(setInt(int)));


    connect(mPreferencesUI->keyboardPaletteKeyButtonSize, qOverload<int>(&QComboBox::currentIndexChanged), settings->boardKeyboardPaletteKeyBtnSize, [=](int index) {
        settings->boardKeyboardPaletteKeyBtnSize->setString(mPreferencesUI->keyboardPaletteKeyButtonSize->itemText(index));
    });
    connect(mPreferencesUI->startModeComboBox, SIGNAL(currentIndexChanged(int)), settings->appStartMode, SLOT(setInt(int)));

    connect(mPreferencesUI->useExternalBrowserCheckBox, SIGNAL(clicked(bool)), settings->webUseExternalBrowser, SLOT(setBool(bool)));
    connect(mPreferencesUI->displayBrowserPageCheckBox, SIGNAL(clicked(bool)), settings->webShowPageImmediatelyOnMirroredScreen, SLOT(setBool(bool)));

    connect(mPreferencesUI->toolbarAtTopRadioButton, SIGNAL(clicked(bool)), this, SLOT(toolbarPositionChanged(bool)));
    connect(mPreferencesUI->toolbarAtBottomRadioButton, SIGNAL(clicked(bool)), this, SLOT(toolbarPositionChanged(bool)));
    // The palette of the tools goes to an edge of the board, as when it is
    // dropped there: at the top or bottom it is horizontal, at the sides vertical.
    mPreferencesUI->horizontalChoice->hide();
    mPreferencesUI->verticalChoice->hide();
    mPaletteEdges = new QButtonGroup(this);
    const QList<QPair<QString, int>> edges = { { tr("Top"), Qt::TopEdge }, { tr("Bottom"), Qt::BottomEdge },
                                               { tr("Left"), Qt::LeftEdge }, { tr("Right"), Qt::RightEdge } };

    for (int i = 0; i < edges.size(); ++i)
    {
        auto choice = new QRadioButton(edges.at(i).first, mPreferencesUI->horizontalChoice->parentWidget());
        mPreferencesUI->gridLayout_5->addWidget(choice, i / 2, i % 2);
        mPaletteEdges->addButton(choice, edges.at(i).second);
    }

    connect(mPaletteEdges, &QButtonGroup::idClicked, this, [](int edge) {
        // The palette of the board and the one of the desktop, as in Bar style.
        UBBarStyle::placeBars(edge);
    });

    // Hidden until the pointer goes near their side, as in Bar style.
    mAutoHideBars = new QCheckBox(tr("Hide automatically"), mPreferencesUI->horizontalChoice->parentWidget());
    mHideEffect = new QComboBox(mPreferencesUI->horizontalChoice->parentWidget());
    mHideEffect->addItem(tr("Slide toward its side"));
    mHideEffect->addItem(tr("Fade out"));
    mPreferencesUI->gridLayout_5->addWidget(mAutoHideBars, 2, 0);
    mPreferencesUI->gridLayout_5->addWidget(mHideEffect, 2, 1);

    // How long the pointer stays away before the bars hide.
    auto delayRow = new QWidget(mPreferencesUI->horizontalChoice->parentWidget());
    auto delayLayout = new QHBoxLayout(delayRow);
    delayLayout->setContentsMargins(0, 0, 0, 0);
    mHideDelay = new QComboBox(delayRow);

    for (int milliseconds : UBBarStyle::hideDelays())
    {
        mHideDelay->addItem(UBBarStyle::hideDelayName(milliseconds), milliseconds);
    }

    delayLayout->addWidget(new QLabel(tr("Wait before hiding"), delayRow));
    delayLayout->addWidget(mHideDelay);
    delayLayout->addStretch(1);
    mPreferencesUI->gridLayout_5->addWidget(delayRow, 3, 0, 1, 2);
    connect(mHideDelay, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        UBBarStyle::setHideDelay(mHideDelay->itemData(index).toInt());
    });

    connect(mAutoHideBars, &QCheckBox::clicked, this, [this](bool on) {
        if (on && !UBBarStyle::askHideEffect(mPreferencesWindow))
        {
            mAutoHideBars->setChecked(false);
            return;
        }

        mHideEffect->setCurrentIndex(int(UBBarStyle::hideEffect()));
        mHideEffect->setEnabled(on);
        mHideDelay->setEnabled(on);
        UBBarStyle::setAutoHide(on);
    });
    connect(mHideEffect, qOverload<int>(&QComboBox::currentIndexChanged), this, [](int index) {
        UBBarStyle::setHideEffect(index == 1 ? UBBarStyle::HideEffect::Fade : UBBarStyle::HideEffect::Slide);
    });
    connect(mPreferencesUI->toolbarDisplayTextCheckBox, SIGNAL(clicked(bool)), settings->appToolBarDisplayText, SLOT(setBool(bool)));

    //grid tab
    //On light background
    QPalette lightBackgroundPalette = QApplication::palette();
    lightBackgroundPalette.setColor(QPalette::Window, Qt::white);

    mPreferencesUI->crossColorLightBackgroundFrame->setAutoFillBackground(true);
    mPreferencesUI->crossColorLightBackgroundFrame->setPalette(lightBackgroundPalette);

    QPalette darkBackgroundPalette = QApplication::palette();
    darkBackgroundPalette.setColor(QPalette::Window, Qt::black);
    darkBackgroundPalette.setColor(QPalette::ButtonText, Qt::white);
    darkBackgroundPalette.setColor(QPalette::WindowText, Qt::white);

    mPreferencesUI->crossColorDarkBackgroundFrame->setAutoFillBackground(true);
    mPreferencesUI->crossColorDarkBackgroundFrame->setPalette(darkBackgroundPalette);
    mPreferencesUI->crossColorDarkBackgroundLabel->setPalette(darkBackgroundPalette);

    QList<QColor> gridLightBackgroundColors = settings->boardGridLightBackgroundColors->colors();
    QColor selectedCrossColorLightBackground(settings->boardCrossColorLightBackground->get().toString());

    mLightBackgroundGridColorPicker = new UBColorPicker(mPreferencesUI->crossColorLightBackgroundFrame);
    mLightBackgroundGridColorPicker->setObjectName(QString::fromUtf8("crossColorLightBackgroundFrame"));
    mLightBackgroundGridColorPicker->setMinimumSize(QSize(32, 32));
    mLightBackgroundGridColorPicker->setFrameShape(QFrame::StyledPanel);
    mLightBackgroundGridColorPicker->setFrameShadow(QFrame::Raised);

    mPreferencesUI->crossColorLightBackgroundLayout->addWidget(mLightBackgroundGridColorPicker);
    mLightBackgroundGridColorPicker->setColors(gridLightBackgroundColors);
    mLightBackgroundGridColorPicker->setSelectedColorIndex(gridLightBackgroundColors.indexOf(selectedCrossColorLightBackground));
    mPreferencesUI->lightBackgroundOpacitySlider->setValue(selectedCrossColorLightBackground.alpha()*100 / 255);

    QObject::connect(mLightBackgroundGridColorPicker, SIGNAL(colorSelected(const QColor&)), this, SLOT(setCrossColorOnLightBackground(const QColor&)));
    connect(mPreferencesUI->lightBackgroundOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(lightBackgroundCrossOpacityValueChanged(int)));

    //On dark background
    QList<QColor> gridDarkBackgroundColors = settings->boardGridDarkBackgroundColors->colors();
    QColor selectedCrossColorDarkBackground(settings->boardCrossColorDarkBackground->get().toString());

    mDarkBackgroundGridColorPicker = new UBColorPicker(mPreferencesUI->crossColorDarkBackgroundFrame);
    mDarkBackgroundGridColorPicker->setObjectName(QString::fromUtf8("crossColorDarkBackgroundFrame"));
    mDarkBackgroundGridColorPicker->setMinimumSize(QSize(32, 32));
    mDarkBackgroundGridColorPicker->setFrameShape(QFrame::StyledPanel);
    mDarkBackgroundGridColorPicker->setFrameShadow(QFrame::Raised);

    mPreferencesUI->crossColorDarkBackgroundLayout->addWidget(mDarkBackgroundGridColorPicker);
    mDarkBackgroundGridColorPicker->setColors(gridDarkBackgroundColors);
    mDarkBackgroundGridColorPicker->setSelectedColorIndex(gridDarkBackgroundColors.indexOf(selectedCrossColorDarkBackground));
    mPreferencesUI->darkBackgroundOpacitySlider->setValue(selectedCrossColorDarkBackground.alpha()*100 / 255);

    QObject::connect(mDarkBackgroundGridColorPicker, SIGNAL(colorSelected(const QColor&)), this, SLOT(setCrossColorOnDarkBackground(const QColor&)));
    connect(mPreferencesUI->darkBackgroundOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(darkBackgroundCrossOpacityValueChanged(int)));

    // pen
    QList<QColor> penLightBackgroundColors = settings->boardPenLightBackgroundColors->colors();
    QList<QColor> penDarkBackgroundColors = settings->boardPenDarkBackgroundColors->colors();
    QList<QColor> penLightBackgroundSelectedColors = settings->boardPenLightBackgroundSelectedColors->colors();
    QList<QColor> penDarkBackgroundSelectedColors = settings->boardPenDarkBackgroundSelectedColors->colors();

    mPenProperties = new UBBrushPropertiesFrame(mPreferencesUI->penFrame,
                                                penLightBackgroundColors, penDarkBackgroundColors, penLightBackgroundSelectedColors,
                                                penDarkBackgroundSelectedColors, this);

    mPenProperties->opacityFrame->hide();

    connect(mPenProperties->fineSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->mediumSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->strongSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->pressureSensitiveCheckBox, SIGNAL(clicked(bool)), settings, SLOT(setPenPressureSensitive(bool)));
    connect(mPenProperties->circleCheckBox, SIGNAL(clicked(bool)), settings, SLOT(setPenPreviewCircle(bool)));
    connect(mPenProperties->circleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(penPreviewFromSizeChanged(int)));

    // marker
    QList<QColor> markerLightBackgroundColors = settings->boardMarkerLightBackgroundColors->colors();
    QList<QColor> markerDarkBackgroundColors = settings->boardMarkerDarkBackgroundColors->colors();
    QList<QColor> markerLightBackgroundSelectedColors = settings->boardMarkerLightBackgroundSelectedColors->colors();
    QList<QColor> markerDarkBackgroundSelectedColors = settings->boardMarkerDarkBackgroundSelectedColors->colors();

    mMarkerProperties = new UBBrushPropertiesFrame(mPreferencesUI->markerFrame, markerLightBackgroundColors,
                                                   markerDarkBackgroundColors, markerLightBackgroundSelectedColors,
                                                   markerDarkBackgroundSelectedColors, this);

    mMarkerProperties->pressureSensitiveCheckBox->setText(tr("Marker is pressure sensitive"));

    mMarkerProperties->circleFrame->hide();

    connect(mMarkerProperties->fineSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->mediumSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->strongSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->pressureSensitiveCheckBox, SIGNAL(clicked(bool)), settings, SLOT(setMarkerPressureSensitive(bool)));
    connect(mMarkerProperties->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));

    // The pen, marker and laser settings are in the options of each tool;
    // only the pressure sensitivity stays here, in the General tab.
    simplifyTabs();
    buildCategories();

    // about tab
    connect(mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox, SIGNAL(clicked(bool)), settings->appEnableAutomaticSoftwareUpdates, SLOT(setBool(bool)));
}

void UBPreferencesController::init()
{
    UBSettings* settings = UBSettings::settings();

    // about tab
    mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox->setChecked(settings->appEnableAutomaticSoftwareUpdates->get().toBool());

    // display tab
    for(int i=0; i<mPreferencesUI->keyboardPaletteKeyButtonSize->count(); i++)
        if (mPreferencesUI->keyboardPaletteKeyButtonSize->itemText(i) == settings->boardKeyboardPaletteKeyBtnSize->get().toString()) {
            mPreferencesUI->keyboardPaletteKeyButtonSize->setCurrentIndex(i);
            break;
        }

    mPreferencesUI->useSystemOSKCheckBox->setChecked(settings->useSystemOnScreenKeyboard->get().toBool());
    this->systemOSKCheckBoxToggled(mPreferencesUI->useSystemOSKCheckBox->isChecked());

    mPreferencesUI->exportBackgroundGrid->setChecked(settings->exportBackgroundGrid->get().toBool());
    mPreferencesUI->exportBackgroundColor->setChecked(settings->exportBackgroundColor->get().toBool());

    mPreferencesUI->showDateColumnOnAlphabeticalSort->setChecked(settings->showDateColumnOnAlphabeticalSort->get().toBool());
    mPreferencesUI->emptyTrashForOlderDocuments->setChecked(settings->emptyTrashForOlderDocuments->get().toBool());
    mPreferencesUI->emptyTrashDaysValue->setValue(settings->emptyTrashDaysValue->get().toInt());

    mPreferencesUI->startModeComboBox->setCurrentIndex(settings->appStartMode->get().toInt());

    mPreferencesUI->useExternalBrowserCheckBox->setChecked(settings->webUseExternalBrowser->get().toBool());
    mPreferencesUI->displayBrowserPageCheckBox->setChecked(settings->webShowPageImmediatelyOnMirroredScreen->get().toBool());
    mPreferencesUI->screenList->loadScreenList(settings->appScreenList->get().toStringList());
    mPreferencesUI->webHomePage->setText(settings->webHomePage->get().toString());

    mPreferencesUI->proxyUsername->setText(settings->proxyUsername());
    mPreferencesUI->proxyPassword->setText(settings->proxyPassword());

    mPreferencesUI->toolbarAtTopRadioButton->setChecked(settings->appToolBarPositionedAtTop->get().toBool());
    mPreferencesUI->toolbarAtBottomRadioButton->setChecked(!settings->appToolBarPositionedAtTop->get().toBool());
    mPreferencesUI->toolbarDisplayTextCheckBox->setChecked(settings->appToolBarDisplayText->get().toBool());
    if (QAbstractButton* edge = mPaletteEdges->button(UBStylusPalette::edge()))
    {
        edge->setChecked(true);
    }

    mAutoHideBars->setChecked(UBBarStyle::autoHide());
    {
        const QSignalBlocker blocker(mHideEffect);
        mHideEffect->setCurrentIndex(int(UBBarStyle::hideEffect()));
    }
    mHideEffect->setEnabled(UBBarStyle::autoHide());
    {
        const QSignalBlocker blocker(mHideDelay);
        mHideDelay->setCurrentIndex(qMax(0, mHideDelay->findData(UBBarStyle::hideDelay())));
    }
    mHideDelay->setEnabled(UBBarStyle::autoHide());

    // pen tab
    mPenProperties->fineSlider->setValue(settings->boardPenFineWidth->get().toDouble() * sSliderRatio);
    mPenProperties->mediumSlider->setValue(settings->boardPenMediumWidth->get().toDouble() * sSliderRatio);
    mPenProperties->strongSlider->setValue(settings->boardPenStrongWidth->get().toDouble() * sSliderRatio);
    mPenProperties->pressureSensitiveCheckBox->setChecked(settings->boardPenPressureSensitive->get().toBool());
    mPenProperties->circleCheckBox->setChecked(settings->showPenPreviewCircle->get().toBool());
    mPenProperties->circleSpinBox->setValue(settings->penPreviewFromSize->get().toInt());

    // marker tab
    mMarkerProperties->fineSlider->setValue(settings->boardMarkerFineWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->mediumSlider->setValue(settings->boardMarkerMediumWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->strongSlider->setValue(settings->boardMarkerStrongWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->pressureSensitiveCheckBox->setChecked(settings->boardMarkerPressureSensitive->get().toBool());

    mMarkerProperties->opacitySlider->setValue(settings->boardMarkerAlpha->get().toDouble() * 100);

}

void UBPreferencesController::close()
{
    applyChanges();
    mPreferencesWindow->accept();
}


void UBPreferencesController::defaultSettings()
{
    UBSettings* settings = UBSettings::settings();

    if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->displayTab)
    {
        mPreferencesUI->screenList->setDefault();

        bool defaultValue = settings->appToolBarPositionedAtTop->reset().toBool();
        mPreferencesUI->toolbarAtTopRadioButton->setChecked(defaultValue);
        mPreferencesUI->toolbarAtBottomRadioButton->setChecked(!defaultValue);

        defaultValue = settings->appToolBarDisplayText->reset().toBool();
        mPreferencesUI->toolbarDisplayTextCheckBox->setChecked(defaultValue);
        // At the right, vertical, as Master Board starts.
        UBBarStyle::placeBars(Qt::RightEdge);
        mPaletteEdges->button(Qt::RightEdge)->setChecked(true);
        UBBarStyle::setAutoHide(false);
        mAutoHideBars->setChecked(false);
        mHideEffect->setEnabled(false);
        mHideDelay->setEnabled(false);
        mPreferencesUI->startModeComboBox->setCurrentIndex(0);

        mPreferencesUI->useSystemOSKCheckBox->setChecked(settings->useSystemOnScreenKeyboard->reset().toBool());

        mPreferencesUI->showDateColumnOnAlphabeticalSort->setChecked(settings->showDateColumnOnAlphabeticalSort->reset().toBool());
        UBApplication::documentController->refreshDateColumns();

        mPreferencesUI->exportBackgroundGrid->setChecked(settings->exportBackgroundGrid->reset().toBool());
        mPreferencesUI->exportBackgroundColor->setChecked(settings->exportBackgroundColor->reset().toBool());

        mPreferencesUI->emptyTrashForOlderDocuments->setChecked(settings->emptyTrashForOlderDocuments->reset().toBool());
        mPreferencesUI->emptyTrashDaysValue->setValue(settings->emptyTrashDaysValue->reset().toInt());

        mPenProperties->pressureSensitiveCheckBox->setChecked(settings->boardPenPressureSensitive->reset().toBool());
        mMarkerProperties->pressureSensitiveCheckBox->setChecked(settings->boardMarkerPressureSensitive->reset().toBool());
    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->penTab)
    {

        mPenProperties->fineSlider->setValue(settings->boardPenFineWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->mediumSlider->setValue(settings->boardPenMediumWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->strongSlider->setValue(settings->boardPenStrongWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->pressureSensitiveCheckBox->setChecked(settings->boardPenPressureSensitive->reset().toBool());
        mPenProperties->circleCheckBox->setChecked(settings->showPenPreviewCircle->reset().toBool());
        mPenProperties->circleSpinBox->setValue(settings->penPreviewFromSize->reset().toInt());

        settings->boardPenLightBackgroundSelectedColors->reset();
        QList<QColor> lightBackgroundSelectedColors = settings->boardPenLightBackgroundSelectedColors->colors();

        settings->boardPenDarkBackgroundSelectedColors->reset();
        QList<QColor> darkBackgroundSelectedColors = settings->boardPenDarkBackgroundSelectedColors->colors();

        for (int i = 0 ; i < settings->colorPaletteSize ; i++)
        {
            mPenProperties->lightBackgroundColorPickers[i]->setSelectedColorIndex(lightBackgroundSelectedColors.indexOf(settings->penColors(false).at(i)));
            mPenProperties->darkBackgroundColorPickers[i]->setSelectedColorIndex(darkBackgroundSelectedColors.indexOf(settings->penColors(true).at(i)));
        }
    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->markerTab)
    {
        mMarkerProperties->fineSlider->setValue(settings->boardMarkerFineWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->mediumSlider->setValue(settings->boardMarkerMediumWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->strongSlider->setValue(settings->boardMarkerStrongWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->pressureSensitiveCheckBox->setChecked(settings->boardMarkerPressureSensitive->reset().toBool());

        mMarkerProperties->opacitySlider->setValue(settings->boardMarkerAlpha->reset().toDouble() * 100);

        settings->boardMarkerLightBackgroundSelectedColors->reset();
        QList<QColor> lightBackgroundSelectedColors = settings->boardMarkerLightBackgroundSelectedColors->colors();

        settings->boardMarkerDarkBackgroundSelectedColors->reset();
        QList<QColor> darkBackgroundSelectedColors = settings->boardMarkerDarkBackgroundSelectedColors->colors();

        for (int i = 0 ; i < settings->colorPaletteSize ; i++)
        {
            mMarkerProperties->lightBackgroundColorPickers[i]->setSelectedColorIndex(lightBackgroundSelectedColors.indexOf(settings->markerColors(false).at(i)));
            mMarkerProperties->darkBackgroundColorPickers[i]->setSelectedColorIndex(darkBackgroundSelectedColors.indexOf(settings->markerColors(true).at(i)));
        }
    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->aboutTab)
    {
        bool defaultValue = settings->appEnableAutomaticSoftwareUpdates->reset().toBool();
        mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox->setChecked(defaultValue);

    }
    else if(mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->networkTab){
        bool defaultValue = settings->webUseExternalBrowser->reset().toBool();
        mPreferencesUI->useExternalBrowserCheckBox->setChecked(defaultValue);
        defaultValue = settings->webShowPageImmediatelyOnMirroredScreen->reset().toBool();
        mPreferencesUI->displayBrowserPageCheckBox->setChecked(defaultValue);

        mPreferencesUI->webHomePage->setText(settings->webHomePage->reset().toString());
    }
    else if(mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->gridTab)
    {
        settings->boardCrossColorDarkBackground->reset();
        settings->boardGridDarkBackgroundColors->reset();

        QList<QColor> gridDarkBackgroundColors = settings->boardGridDarkBackgroundColors->colors();
        QColor selectedCrossColorDarkBackground(settings->boardCrossColorDarkBackground->get().toString());

        mDarkBackgroundGridColorPicker->setColors(gridDarkBackgroundColors);
        mDarkBackgroundGridColorPicker->setSelectedColorIndex(gridDarkBackgroundColors.indexOf(selectedCrossColorDarkBackground));
        int darkBackgroundOpacity = selectedCrossColorDarkBackground.alpha()*100 / 255;
        mPreferencesUI->darkBackgroundOpacitySlider->setValue(darkBackgroundOpacity);
        darkBackgroundCrossOpacityValueChanged(darkBackgroundOpacity);

        settings->boardCrossColorLightBackground->reset();
        settings->boardGridLightBackgroundColors->reset();

        QList<QColor> gridLightBackgroundColors = settings->boardGridLightBackgroundColors->colors();
        QColor selectedCrossColorLightBackground(settings->boardCrossColorLightBackground->get().toString());

        mLightBackgroundGridColorPicker->setColors(gridLightBackgroundColors);
        mLightBackgroundGridColorPicker->setSelectedColorIndex(gridLightBackgroundColors.indexOf(selectedCrossColorLightBackground));
        int lightBackgroundOpacity = selectedCrossColorLightBackground.alpha()*100 / 255;
        mPreferencesUI->lightBackgroundOpacitySlider->setValue(lightBackgroundOpacity);
        lightBackgroundCrossOpacityValueChanged(lightBackgroundOpacity);

    }
}

void UBPreferencesController::darkBackgroundCrossOpacityValueChanged(int value)
{
    UBSettings* settings = UBSettings::settings();
    int opacity = value * 255 / 100;

    QList<QColor> gridDarkBackgroundColors = settings->boardGridDarkBackgroundColors->colors();

    int index = mDarkBackgroundGridColorPicker->selectedColorIndex();
    QColor currentColor = gridDarkBackgroundColors.at(index);
    currentColor.setAlpha(opacity);
    gridDarkBackgroundColors.replace(index, currentColor);
    mDarkBackgroundGridColorPicker->setColors(gridDarkBackgroundColors);

    settings->boardGridDarkBackgroundColors->setColor(index, currentColor);

    UBSettings::settings()->boardCrossColorDarkBackground->set(currentColor.name(QColor::HexArgb));

    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        foreach(QGraphicsView* view, UBApplication::boardController->activeScene()->views())
            view->resetCachedContent();
    }
}

void UBPreferencesController::lightBackgroundCrossOpacityValueChanged(int value)
{
    UBSettings* settings = UBSettings::settings();
    int opacity = value * 255 / 100;

    QList<QColor> gridLightBackgroundColors = settings->boardGridLightBackgroundColors->colors();

    int index = mLightBackgroundGridColorPicker->selectedColorIndex();
    QColor currentColor = gridLightBackgroundColors.at(index);
    currentColor.setAlpha(opacity);
    gridLightBackgroundColors.replace(index, currentColor);
    mLightBackgroundGridColorPicker->setColors(gridLightBackgroundColors);

    settings->boardGridLightBackgroundColors->setColor(index, currentColor);

    UBSettings::settings()->boardCrossColorLightBackground->set(currentColor.name(QColor::HexArgb));

    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        foreach(QGraphicsView* view, UBApplication::boardController->activeScene()->views())
            view->resetCachedContent();
    }
}

void UBPreferencesController::penPreviewFromSizeChanged(int value)
{
    UBSettings::settings()->setPenPreviewFromSize(value);
}

void UBPreferencesController::widthSliderChanged(int value)
{
    UBSettings* settings = UBSettings::settings();

    qreal width = value / sSliderRatio;

    QObject *slider = sender();
    UBCircleFrame* display = 0;

    if (slider == mPenProperties->fineSlider)
    {
        settings->boardPenFineWidth->set(width);
        display = mPenProperties->fineDisplayFrame;
    }
    else if (slider == mPenProperties->mediumSlider)
    {
        settings->boardPenMediumWidth->set(width);
        display = mPenProperties->mediumDisplayFrame;
    }
    else if (slider == mPenProperties->strongSlider)
    {
        settings->boardPenStrongWidth->set(width);
        display = mPenProperties->strongDisplayFrame;
    }
    else if (slider == mMarkerProperties->fineSlider)
    {
        settings->boardMarkerFineWidth->set(width);
        display = mMarkerProperties->fineDisplayFrame;
    }
    else if (slider == mMarkerProperties->mediumSlider)
    {
        settings->boardMarkerMediumWidth->set(width);
        display = mMarkerProperties->mediumDisplayFrame;
    }
    else if (slider == mMarkerProperties->strongSlider)
    {
        settings->boardMarkerStrongWidth->set(width);
        display = mMarkerProperties->strongDisplayFrame;
    }
    else
    {
        qDebug() << "widthSliderChanged : unknown sender ... ignoring event";
    }

    if (display)
    {
        display->currentPenWidth = width;
        display->maxPenWidth = sMaxPenWidth;
        display->repaint();
    }
}


void UBPreferencesController::opacitySliderChanged(int value)
{
    qreal opacity = ((qreal)value) / 100;

    QObject *slider = sender();

    if (slider == mMarkerProperties->opacitySlider)
    {
        UBDrawingController::drawingController()->setMarkerAlpha(opacity);
    }
}


void UBPreferencesController::colorSelected(const QColor& color)
{
    UBColorPicker *colorPicker = qobject_cast<UBColorPicker*>(sender());

    int index = mPenProperties->lightBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setPenColor(false, color, index);
        return;
    }

    index = mPenProperties->darkBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setPenColor(true, color, index);
    }

    index = mMarkerProperties->lightBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setMarkerColor(false, color, index);
    }

    index = mMarkerProperties->darkBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setMarkerColor(true, color, index);
    }

}

void UBPreferencesController::setCrossColorOnDarkBackground(const QColor& color)
{
    UBSettings::settings()->boardCrossColorDarkBackground->set(color.name(QColor::HexArgb));

    mPreferencesUI->darkBackgroundOpacitySlider->setValue(color.alpha() * 100 / 255);

    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        foreach(QGraphicsView* view, UBApplication::boardController->activeScene()->views())
            view->resetCachedContent();
    }
}

void UBPreferencesController::setCrossColorOnLightBackground(const QColor& color)
{
    UBSettings::settings()->boardCrossColorLightBackground->set(color.name(QColor::HexArgb));

    mPreferencesUI->lightBackgroundOpacitySlider->setValue(color.alpha() * 100 / 255);

    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        foreach(QGraphicsView* view, UBApplication::boardController->activeScene()->views())
            view->resetCachedContent();
    }
}

void UBPreferencesController::toolbarPositionChanged(bool checked)
{
    Q_UNUSED(checked);

    UBSettings* settings = UBSettings::settings();

    settings->appToolBarPositionedAtTop->set(mPreferencesUI->toolbarAtTopRadioButton->isChecked());

}

void UBPreferencesController::toolbarOrientationVertical(bool checked)
{
    UBSettings* settings = UBSettings::settings();
    settings->appToolBarOrientationVertical->set(checked);
}

void UBPreferencesController::toolbarOrientationHorizontal(bool checked)
{
    UBSettings* settings = UBSettings::settings();
    settings->appToolBarOrientationVertical->set(!checked);
}

void UBPreferencesController::systemOSKCheckBoxToggled(bool checked)
{
    mPreferencesUI->keyboardPaletteKeyButtonSize->setVisible(!checked);
    mPreferencesUI->keyboardPaletteKeyButtonSize_Label->setVisible(!checked);
}

UBBrushPropertiesFrame::UBBrushPropertiesFrame(QFrame* owner, const QList<QColor>& lightBackgroundColors,
                                               const QList<QColor>& darkBackgroundColors, const QList<QColor>& lightBackgroundSelectedColors,
                                               const QList<QColor>& darkBackgroundSelectedColors, UBPreferencesController* controller)
{
    setupUi(owner);

    QPalette lightBackgroundPalette = QApplication::palette();
    lightBackgroundPalette.setColor(QPalette::Window, Qt::white);

    lightBackgroundFrame->setAutoFillBackground(true);
    lightBackgroundFrame->setPalette(lightBackgroundPalette);

    QPalette darkBackgroundPalette = QApplication::palette();
    darkBackgroundPalette.setColor(QPalette::Window, Qt::black);
    darkBackgroundPalette.setColor(QPalette::ButtonText, Qt::white);
    darkBackgroundPalette.setColor(QPalette::WindowText, Qt::white);

    darkBackgroundFrame->setAutoFillBackground(true);
    darkBackgroundFrame->setPalette(darkBackgroundPalette);
    darkBackgroundLabel->setPalette(darkBackgroundPalette);

    QList<QColor> firstLightBackgroundColor;
    firstLightBackgroundColor.append(lightBackgroundColors[0]);

    lightBackgroundColorPicker0->setColors(firstLightBackgroundColor);
    lightBackgroundColorPicker0->setSelectedColorIndex(0);
    lightBackgroundColorPickers.append(lightBackgroundColorPicker0);

    for (int i = 1 ; i < UBSettings::settings()->colorPaletteSize ; i++)
    {
        UBColorPicker *picker = new UBColorPicker(lightBackgroundFrame);
        picker->setObjectName(QString("penLightBackgroundColor%1").arg(i));
        picker->setMinimumSize(QSize(32, 32));
        picker->setFrameShape(QFrame::StyledPanel);
        picker->setFrameShadow(QFrame::Raised);

        lightBackgroundLayout->addWidget(picker);

        picker->setColors(lightBackgroundColors);

        picker->setSelectedColorIndex(lightBackgroundColors.indexOf(lightBackgroundSelectedColors.at(i)));

        lightBackgroundColorPickers.append(picker);

        QObject::connect(picker, SIGNAL(colorSelected(const QColor&)), controller, SLOT(colorSelected(const QColor&)));

    }

    QList<QColor> firstDarkBackgroundColor;
    firstDarkBackgroundColor.append(darkBackgroundColors[0]);

    darkBackgroundColorPicker0->setColors(firstDarkBackgroundColor);
    darkBackgroundColorPicker0->setSelectedColorIndex(0);
    darkBackgroundColorPickers.append(darkBackgroundColorPicker0);

    for (int i = 1 ; i < UBSettings::settings()->colorPaletteSize ; i++)
    {
        UBColorPicker *picker = new UBColorPicker(darkBackgroundFrame);
        picker->setObjectName(QString("penDarkBackgroundColor%1").arg(i));
        picker->setMinimumSize(QSize(32, 32));
        picker->setFrameShape(QFrame::StyledPanel);
        picker->setFrameShadow(QFrame::Raised);

        darkBackgroundLayout->addWidget(picker);

        picker->setColors(darkBackgroundColors);
        picker->setSelectedColorIndex(darkBackgroundColors.indexOf(darkBackgroundSelectedColors.at(i)));

        darkBackgroundColorPickers.append(picker);

        QObject::connect(picker, SIGNAL(colorSelected(const QColor&)), controller, SLOT(colorSelected(const QColor&)));

    }
}

UBScreenListLineEdit::UBScreenListLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , mValidator(nullptr)
{
    connect(this, &QLineEdit::textChanged, this, &UBScreenListLineEdit::onTextChanged);
}

void UBScreenListLineEdit::setDefault()
{
    setText("");
}

void UBScreenListLineEdit::loadScreenList(const QStringList &screenList)
{
    setText(screenList.join(','));
}

void UBScreenListLineEdit::focusInEvent(QFocusEvent *focusEvent)
{
    QLineEdit::focusInEvent(focusEvent);

    if (mScreenLabels.empty())
    {
        QStringList screenList = UBStringUtils::trimmed(text().split(','));

        QList<QScreen*> screens = UBApplication::displayManager->availableScreens();
        QStringList availableScreenIndexes;
        int screenIndex = 1;
        QFont font;
        font.setPointSize(48);

        for (QScreen* screen : screens)
        {
            QString index = QString::number(screenIndex);
            availableScreenIndexes << index;

            QPushButton* button = new QPushButton(this);
            button->setWindowFlag(Qt::FramelessWindowHint, true);
            button->setWindowFlag(Qt::WindowStaysOnTopHint, true);
            button->setWindowFlag(Qt::X11BypassWindowManagerHint, true);
            button->setWindowFlag(Qt::Window, true);
            button->setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);
            button->setAttribute(Qt::WA_ShowWithoutActivating, true);
            button->setProperty("screenIndex", index);
#ifdef QT_DEBUG
            button->setText(index + "(" + screen->name() + ")");
#else
            button->setText(index);
#endif
            button->setFont(font);
            button->move(screen->geometry().topLeft());
            button->setMinimumSize(300, 150);
            button->setDisabled(screenList.contains(index));
            button->setCursor(Qt::PointingHandCursor);
            button->show();

            connect(button, &QPushButton::pressed, this, &UBScreenListLineEdit::addScreen);

            mScreenLabels << button;
            ++screenIndex;
        }

        if (!mValidator)
        {
            mValidator = new UBStringListValidator(this);
            setValidator(mValidator);
        }

        mValidator->setValidationStringList(availableScreenIndexes);
    }
}

void UBScreenListLineEdit::focusOutEvent(QFocusEvent *focusEvent)
{
    QLineEdit::focusOutEvent(focusEvent);
    qDeleteAll(mScreenLabels);
    mScreenLabels.clear();
}

void UBScreenListLineEdit::addScreen()
{
    QPushButton* button = dynamic_cast<QPushButton*>(sender());

    if (button)
    {
        QString list = text();
        mValidator->fixup(list);
        QString screenIndex = button->property("screenIndex").toString();

        if (list.isEmpty())
        {
            setText(screenIndex);
        }
        else
        {
            setText(list + "," + screenIndex);
        }

        button->setEnabled(false);
    }
}

void UBScreenListLineEdit::onTextChanged(const QString &input)
{
    const QStringList screenList = UBStringUtils::trimmed(input.split(','));

    for (QPushButton* button : std::as_const(mScreenLabels))
    {
        button->setDisabled(screenList.contains(button->property("screenIndex").toString()));
    }

    if (input.isEmpty() || input.right(1) == ',')
    {
        // create and attach a new QCompleter
        QStringList model;

        for (QPushButton* button : std::as_const(mScreenLabels))
        {
            if (button->isEnabled())
            {
                model << input + button->property("screenIndex").toString();
            }
        }
    }

    // user indication of acceptable input
    if (hasAcceptableInput())
    {
        setStyleSheet("");
        emit screenListChanged(screenList);
    }
    else
    {
        setStyleSheet("QLineEdit { background-color: #FFB3C8; }");
    }
}

UBStringListValidator::UBStringListValidator(QObject *parent)
    : QValidator(parent)
{

}

void UBStringListValidator::fixup(QString &input) const
{
    // remove invalid tokens from list, trim tokens
    QStringList inputList = UBStringUtils::trimmed(input.split(','));
    QStringList outputList;

    for (const QString& token : inputList)
    {
        if (mList.contains(token))
        {
            outputList << token;
        }
    }

    input = outputList.join(',');
}

QValidator::State UBStringListValidator::validate(QString &input, int &) const
{
    bool ok = true;
    QStringList inputList = UBStringUtils::trimmed(input.split(','));
    // number of commas must match number of list items - 1
    int commas = input.count(',');

    if (commas && commas + 1 != inputList.size())
    {
        ok = false;
    }

    for (const QString& token : inputList)
    {
        ok &= mList.contains(token) && inputList.count(token) == 1;
    }

    return ok ? Acceptable : Intermediate;
}

void UBStringListValidator::setValidationStringList(const QStringList &list)
{
    mList = list;
}
