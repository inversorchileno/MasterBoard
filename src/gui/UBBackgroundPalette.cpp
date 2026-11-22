#include "UBBackgroundPalette.h"

#include <QCheckBox>
#include <QDateEdit>
#include <QGridLayout>
#include <QPainter>
#include <QGraphicsView>
#include <QSlider>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBPageTemplates.h"
#include "frameworks/UBPlatformUtils.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolOptionsWidgets.h"

static QList<QColor> pageColors()
{
    // White and black first, then light colors to write on and two dark ones.
    return { QColor("#ffffff"), QColor("#000000"), QColor("#fdf6e3"), QColor("#fff7b0"), QColor("#dbeafe"),
             QColor("#dcfce7"), QColor("#fde2ec"), QColor("#1e3a5f"), QColor("#1f4d2b") };
}

// The drawing of a way the pages follow each other: the page in the middle,
// and the edges of the pages around it, as in the page settings of Samsung Notes.
static QIcon pageFlowIcon(UBBoardView::PageFlow flow)
{
    QPixmap pixmap(48, 28);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#4b5563"), 1.6));

    auto sheet = [&painter](const QRectF& rect, bool middle) {
        painter.setBrush(middle ? QColor("#f59e5b") : Qt::transparent);
        painter.drawRoundedRect(rect, 2, 2);
    };

    switch (flow)
    {
    case UBBoardView::VerticalFlow:
        sheet(QRectF(18.5, -5, 11, 8), false);
        sheet(QRectF(18.5, 7.5, 11, 13), true);
        sheet(QRectF(18.5, 24.5, 11, 8), false);
        break;

    case UBBoardView::HorizontalFlow:
        sheet(QRectF(-4, 7.5, 9, 13), false);
        sheet(QRectF(18.5, 7.5, 11, 13), true);
        sheet(QRectF(43, 7.5, 9, 13), false);
        break;

    case UBBoardView::TwoPagesFlow:
        sheet(QRectF(-6, 7.5, 9, 13), false);
        sheet(QRectF(13, 7.5, 11, 13), true);
        sheet(QRectF(24, 7.5, 11, 13), true);
        sheet(QRectF(45, 7.5, 9, 13), false);
        break;
    }

    return QIcon(pixmap);
}

UBBackgroundPalette::UBBackgroundPalette(QList<QAction*> actions, QWidget * parent)
    : UBActionPalette(parent)
{
    init();
    setActions(actions);
}


UBBackgroundPalette::UBBackgroundPalette(QWidget * parent)
     : UBActionPalette(parent)
{
    init();
}



void UBBackgroundPalette::init()
{
    UBActionPalette::clearLayout();
    delete layout();


    m_customCloseProcessing = false;

    mButtonSize = QSize(32, 32);
    mIsClosable = false;
    mAutoClose = false;
    mToolButtonStyle = Qt::ToolButtonIconOnly;
    mButtons.clear();

    mVLayout = new QVBoxLayout(this);
    mTopLayout = new QHBoxLayout();
    mBottomLayout = new QHBoxLayout();

    mColorLayout = new QHBoxLayout();

    // Two tabs: the patterns of the page, and the templates. The page color is for both.
    mTabs = new UBSegmentedChoice({ tr("Page"), tr("Templates") }, this);
    mTabs->setChoice(0);
    connect(mTabs, &UBSegmentedChoice::choiceChanged, this, [this](int tab) { showTemplates(tab == 1); });
    mVLayout->addWidget(mTabs);
    mVLayout->addLayout(mColorLayout);

    // The patterns and the grid size, in a box of their own: their buttons
    // follow the state of their actions, the box follows the tab.
    mPatterns = new QWidget(this);
    auto patternsLayout = new QVBoxLayout(mPatterns);
    patternsLayout->setContentsMargins(0, 0, 0, 0);
    patternsLayout->addLayout(mTopLayout);
    patternsLayout->addLayout(mBottomLayout);
    patternsLayout->addLayout(createLineColor());
    mVLayout->addWidget(mPatterns);

    mTemplates = createTemplates();
    mTemplates->hide();
    mVLayout->addWidget(mTemplates);

    mVLayout->addLayout(createPageFlow());

    // Fully opaque, so that the page under it does not show through.
    setBackgroundBrush(UBSettings::opaquePaletteColor);

    // Any color for the page, with the pattern of the template chosen above.
    mColorLabel = new QLabel(tr("Page color"));
    mColorLabel->setStyleSheet("QLabel { color : white; }");
    mPageColors = new UBColorSwatches(pageColors(), false, this);
    mColorLayout->addSpacing(16);
    mColorLayout->addWidget(mColorLabel);
    mColorLayout->addWidget(mPageColors);
    mColorLayout->addStretch(1);

    connect(mPageColors, &UBColorSwatches::colorChosen, this, [](const QColor& color) {
        UBApplication::boardController->changeBackgroundColor(color);
    });

    mSlider = new QSlider(Qt::Horizontal);

    mSlider->setMinimum(UBSettings::settings()->minCrossSize);
    mSlider->setMaximum(UBSettings::settings()->maxCrossSize);
    mSlider->setSingleStep(2);
    mSlider->setTracking(true); // valueChanged() is emitted during movement and not just upon releasing the slider

    mSliderLabel = new QLabel(tr("Grid size"));
    mIntermediateLinesLabel = new QLabel(tr("Draw intermediate grid lines"));

    mResetDefaultGridSizeButton = createPaletteButton(UBApplication::mainWindow->actionDefaultGridSize, this);
    mResetDefaultGridSizeButton->setFixedSize(24,24);
    mActions << UBApplication::mainWindow->actionDefaultGridSize;

    connect(UBApplication::mainWindow->actionDefaultGridSize, SIGNAL(triggered()), this, SLOT(defaultBackgroundGridSize()));

    mDrawIntermediateLinesCheckBox = createPaletteButton(UBApplication::mainWindow->actionDrawIntermediateGridLines, this);
    mDrawIntermediateLinesCheckBox->setFixedSize(24,24);
    mDrawIntermediateLinesCheckBox->setCheckable(true);
    mActions << UBApplication::mainWindow->actionDrawIntermediateGridLines;
    UBApplication::mainWindow->actionDrawIntermediateGridLines->setProperty("ungrouped", true);
    connect(UBApplication::mainWindow->actionDrawIntermediateGridLines, SIGNAL(toggled(bool)), this, SLOT(toggleIntermediateLines(bool)));

    mBottomLayout->addSpacing(16);
    mBottomLayout->addWidget(mSliderLabel);
    mBottomLayout->addWidget(mSlider);
    mBottomLayout->addWidget(mResetDefaultGridSizeButton);
    mBottomLayout->addSpacing(16);

    mBottomLayout->addWidget(mIntermediateLinesLabel);
    mBottomLayout->addWidget(mDrawIntermediateLinesCheckBox);
    mBottomLayout->addSpacing(16);

    updateLayout();
}

// The language of Master Board: the one chosen in the preferences, or the one of the system.
static QString applicationLanguage()
{
    const QString language = UBSettings::settings()->appPreferredLanguage->get().toString();
    return language.isEmpty() ? UBPlatformUtils::systemLanguage() : language;
}

QWidget* UBBackgroundPalette::createTemplates()
{
    auto templates = new QWidget(this);
    auto layout = new QVBoxLayout(templates);
    layout->setContentsMargins(16, 0, 16, 0);

    // The date of the calendars and planners.
    auto dateRow = new QHBoxLayout();
    auto dateLabel = new QLabel(tr("Date of the calendars"), templates);
    dateLabel->setStyleSheet("QLabel { color : white; }");
    mTemplateDate = new QDateEdit(QDate::currentDate(), templates);
    mTemplateDate->setCalendarPopup(true);
    mTemplateDate->setDisplayFormat("dd/MM/yyyy");
    mTemplateDate->setToolTip(tr("The calendars and planners are made for this day, its week, its month or its year"));
    // Monday first, as in most of the world; Sunday for those who use it.
    mSundayFirst = new QCheckBox(tr("Weeks start on Sunday"), templates);
    mSundayFirst->setStyleSheet("QCheckBox { color : white; }");
    mSundayFirst->setChecked(UBSettings::settings()->weekStartsOnSunday->get().toBool());
    mSundayFirst->setToolTip(tr("The calendars start their weeks on Monday, or on Sunday when this is ticked"));
    connect(mSundayFirst, &QCheckBox::toggled, this, [this](bool sunday) {
        UBSettings::settings()->weekStartsOnSunday->set(sunday);
        drawTemplatePictures();
    });

    dateRow->addWidget(dateLabel);
    dateRow->addWidget(mTemplateDate);
    dateRow->addSpacing(16);
    dateRow->addWidget(mSundayFirst);
    dateRow->addStretch(1);
    layout->addLayout(dateRow);

    auto grid = new QGridLayout();
    grid->setSpacing(6);
    layout->addLayout(grid);

    // A picture of each template on a white page, with its name under it.
    const UBPageTemplates pageTemplates(applicationLanguage());
    const QSize thumbnail(120, 68);
    const QList<UBPageTemplates::Template> all = UBPageTemplates::all();

    for (int i = 0; i < all.size(); ++i)
    {
        const UBPageTemplates::Template pageTemplate = all.at(i);
        auto button = new QToolButton(templates);
        const QString name = pageTemplates.name(pageTemplate);
        mTemplateButtons.insert(pageTemplate, button);
        button->setIconSize(thumbnail);
        button->setText(button->fontMetrics().elidedText(name, Qt::ElideRight, thumbnail.width()));
        button->setToolTip(UBPageTemplates::isDated(pageTemplate)
                               ? tr("%1, for the date chosen above").arg(name)
                               : name);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setFixedWidth(thumbnail.width() + 12);
        button->setStyleSheet("QToolButton { color: white; border: 1px solid transparent; border-radius: 4px; padding: 2px; }"
                              "QToolButton:hover { border-color: #8a8f99; background: rgba(255, 255, 255, 30); }");
        grid->addWidget(button, i / 7, i % 7);

        connect(button, &QToolButton::clicked, this, [this, pageTemplate]() {
            UBApplication::boardController->applyPageTemplate(pageTemplate, mTemplateDate->date());
        });
    }

    auto hint = new QLabel(tr("On a page of a PDF, the template goes on a new page after it."), templates);
    hint->setStyleSheet("QLabel { color : #c9ced6; }");
    layout->addWidget(hint);

    drawTemplatePictures();
    return templates;
}

void UBBackgroundPalette::drawTemplatePictures()
{
    UBPageTemplates pageTemplates(applicationLanguage());
    pageTemplates.setWeekStartsOnSunday(UBSettings::settings()->weekStartsOnSunday->get().toBool());
    const QSize thumbnail(120, 68);
    const qreal ratio = 2;

    for (auto it = mTemplateButtons.cbegin(); it != mTemplateButtons.cend(); ++it)
    {
        QPixmap picture(thumbnail * ratio);
        picture.setDevicePixelRatio(ratio);
        picture.fill(Qt::white);
        QPainter painter(&picture);
        pageTemplates.draw(&painter, QRectF(QPointF(0, 0), thumbnail), it.key(), QDate::currentDate());
        painter.end();
        it.value()->setIcon(QIcon(picture));
    }
}

QLayout* UBBackgroundPalette::createLineColor()
{
    // The color and the opacity of the lines of the grid, of the ruled page
    // and of the dots: for a light page, or for a dark one, as the current page.
    auto row = new QHBoxLayout();
    mLineColorLabel = new QLabel(tr("Color of the lines"), this);
    mLineColorLabel->setStyleSheet("QLabel { color : white; }");

    const QList<QColor> colors = { QColor("#A5E1FF"), QColor("#C0C0C0"), QColor("#808080"), QColor("#000000"),
                                   QColor("#ffffff"), QColor("#93c5fd"), QColor("#86efac"), QColor("#fca5a5"),
                                   QColor("#fcd34d") };
    mLineColors = new UBColorSwatches(colors, false, this);
    mLineColors->hideOtherColorButton();

    mLineOpacityLabel = new QLabel(tr("Opacity"), this);
    mLineOpacityLabel->setStyleSheet("QLabel { color : white; }");
    mLineOpacity = new QSlider(Qt::Horizontal, this);
    mLineOpacity->setRange(10, 100);
    mLineOpacity->setFixedWidth(120);

    row->addSpacing(16);
    row->addWidget(mLineColorLabel);
    row->addWidget(mLineColors);
    row->addSpacing(16);
    row->addWidget(mLineOpacityLabel);
    row->addWidget(mLineOpacity);
    row->addStretch(1);

    auto apply = [this]() {
        QColor color = mLineColors->currentColor();

        if (!color.isValid())
        {
            return;
        }

        color.setAlpha(mLineOpacity->value() * 255 / 100);
        lineColorSetting()->set(color.name(QColor::HexArgb));

        if (UBApplication::boardController->activeScene())
        {
            // The views keep their background in a cache.
            for (QGraphicsView* view : UBApplication::boardController->activeScene()->views())
            {
                view->resetCachedContent();
            }
        }
    };

    connect(mLineColors, &UBColorSwatches::colorChosen, this, apply);
    connect(mLineOpacity, &QSlider::valueChanged, this, apply);

    return row;
}

QLayout* UBBackgroundPalette::createPageFlow()
{
    // How the pages follow each other: under, beside, or two by two.
    auto row = new QHBoxLayout();
    auto label = new QLabel(tr("Scroll direction"), this);
    label->setStyleSheet("QLabel { color : white; }");

    mPageFlow = new UBSegmentedChoice({ tr("Vertical"), tr("Horizontal"), tr("Horizontal in 2 pages") }, this);
    mPageFlow->setIcons({ pageFlowIcon(UBBoardView::VerticalFlow), pageFlowIcon(UBBoardView::HorizontalFlow),
                          pageFlowIcon(UBBoardView::TwoPagesFlow) });
    mPageFlow->setToolTip(tr("The pages one under the other, one beside the other, or two by two as an open notebook"));
    mPageFlow->setChoice(UBBoardView::pageFlow());

    connect(mPageFlow, &UBSegmentedChoice::choiceChanged, this, [](int choice) {
        if (UBBoardView* view = UBApplication::boardController->controlView())
        {
            view->setPageFlow(UBBoardView::PageFlow(choice));
        }
    });

    row->addSpacing(16);
    row->addWidget(label);
    row->addWidget(mPageFlow);
    row->addStretch(1);
    return row;
}

UBSetting* UBBackgroundPalette::lineColorSetting()
{
    UBSettings* settings = UBSettings::settings();
    const auto scene = UBApplication::boardController->activeScene();
    return scene && scene->isDarkBackground() ? settings->boardCrossColorDarkBackground
                                               : settings->boardCrossColorLightBackground;
}

void UBBackgroundPalette::showLineColor()
{
    const QColor color(lineColorSetting()->get().toString());
    const QSignalBlocker colorBlocker(mLineColors);
    const QSignalBlocker opacityBlocker(mLineOpacity);
    mLineColors->setCurrentColor(QColor(color.red(), color.green(), color.blue()));
    mLineOpacity->setValue(qMax(10, color.alpha() * 100 / 255));
}

void UBBackgroundPalette::showTemplates(bool shown)
{
    mPatterns->setVisible(!shown);
    mTemplates->setVisible(shown);
    adjustSize();
}

void UBBackgroundPalette::addAction(QAction* action)
{
    UBActionPaletteButton* button = createPaletteButton(action, mPatterns);

    mTopLayout->addWidget(button);
    mActions << action;
}

void UBBackgroundPalette::setActions(QList<QAction*> actions)
{
    mMapActionToButton.clear();

    foreach(QAction* action, actions)
    {
        addAction(action);
    }

    actionChanged();
}

void UBBackgroundPalette::updateLayout()
{
    if (mToolButtonStyle == Qt::ToolButtonIconOnly) {
        mVLayout->setContentsMargins (sLayoutContentMargin / 2  + border(), sLayoutContentMargin / 2  + border()
                , sLayoutContentMargin / 2  + border(), sLayoutContentMargin / 2  + border());
    }
    else
    {
        mVLayout->setContentsMargins (sLayoutContentMargin  + border(), sLayoutContentMargin  + border()
                , sLayoutContentMargin  + border(), sLayoutContentMargin + border());

    }
   update();
}

void UBBackgroundPalette::clearLayout()
{
    while(!mTopLayout->isEmpty()) {
        QLayoutItem* pItem = mTopLayout->itemAt(0);
        QWidget* pW = pItem->widget();
        mTopLayout->removeItem(pItem);
        delete pItem;
        mTopLayout->removeWidget(pW);
        delete pW;
    }

    delete mTopLayout;

    while(!mBottomLayout->isEmpty()) {
        QLayoutItem* pItem = mBottomLayout->itemAt(0);
        QWidget* pW = pItem->widget();
        mBottomLayout->removeItem(pItem);
        delete pItem;
        mBottomLayout->removeWidget(pW);
        delete pW;
    }

    delete mBottomLayout;

    while(!mColorLayout->isEmpty()) {
        QLayoutItem* pItem = mColorLayout->takeAt(0);
        delete pItem->widget();
        delete pItem;
    }

    delete mColorLayout;

    delete mTabs;
    mTabs = nullptr;
    delete mPatterns;
    mPatterns = nullptr;
    delete mTemplates;
    mTemplates = nullptr;

    delete mVLayout;

    mActions.clear();
    mButtons.clear();
}

void UBBackgroundPalette::showEvent(QShowEvent* event)
{
    backgroundChanged();

    mSlider->setValue(UBApplication::boardController->activeScene()->backgroundGridSize());
    connect(mSlider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderValueChanged(int)));

    mDrawIntermediateLinesCheckBox->setChecked(UBApplication::boardController->activeScene()->intermediateLines());

    QWidget::showEvent(event);
}

void UBBackgroundPalette::sliderValueChanged(int value)
{
    UBApplication::boardController->activeScene()->setBackgroundGridSize(value);
    UBSettings::settings()->crossSize = value; // since this function is called (indirectly, by refresh) when we switch scenes, the settings will always have the current scene's cross size.

    UBApplication::boardController->activeScene()->setModified(true);
}

void UBBackgroundPalette::defaultBackgroundGridSize()
{
    mSlider->setValue(UBSettings::settings()->defaultCrossSize);
    sliderValueChanged(UBSettings::settings()->defaultCrossSize);
}

void UBBackgroundPalette::toggleIntermediateLines(bool checked)
{
    UBApplication::boardController->activeScene()->setIntermediateLines(checked);
    UBSettings::settings()->intermediateLines = checked; // since this function is called (indirectly, by refresh) when we switch scenes, the settings will always have the current scene's value.
}

void UBBackgroundPalette::backgroundChanged()
{
    // The palette is opaque and dark, whatever the page under it.
    mSliderLabel->setStyleSheet("QLabel { color : white; }");
    mIntermediateLinesLabel->setStyleSheet("QLabel { color : white; }");
    mPageColors->setCurrentColor(UBApplication::boardController->activeScene()->backgroundColor());

    // A dark page has its own color of the lines.
    if (mLineColors)
    {
        showLineColor();
    }
}

void UBBackgroundPalette::refresh()
{
    backgroundChanged();
    mSlider->setValue(UBApplication::boardController->activeScene()->backgroundGridSize());
}
