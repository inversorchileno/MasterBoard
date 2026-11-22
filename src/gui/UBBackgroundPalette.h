#ifndef UBBACKGROUNDPALETTE_H
#define UBBACKGROUNDPALETTE_H

#include "gui/UBActionPalette.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBPageTemplates.h"

class QCheckBox;
class QDateEdit;
class QToolButton;
class UBColorSwatches;
class UBSetting;
class UBSegmentedChoice;

class UBBackgroundPalette : public UBActionPalette
{
    Q_OBJECT

    public:

        UBBackgroundPalette(QList<QAction*> actions, QWidget* parent = 0);
        UBBackgroundPalette(QWidget* parent = 0);

        void addAction(QAction *action);
        void setActions(QList<QAction *> actions);
        void clearLayout();


    public slots:
        void showEvent(QShowEvent* event);
        void backgroundChanged();
        void refresh();

    protected slots:
        void sliderValueChanged(int value);
        void defaultBackgroundGridSize();
        void toggleIntermediateLines(bool checked);

    protected:
        virtual void updateLayout();
        void init();

    private:
        QWidget* createTemplates();
        void drawTemplatePictures();
        void showTemplates(bool shown);
        void applyLook();
        QLayout* createLineColor();

        /** @brief The choice of how the pages follow each other: under, beside, or two by two. */
        QLayout* createPageFlow();

        /** @brief The setting of the color of the lines for the background of the page: light or dark. */
        static UBSetting* lineColorSetting();
        void showLineColor();


        QVBoxLayout* mVLayout;
        QHBoxLayout* mTopLayout;
        QHBoxLayout* mBottomLayout;
        QHBoxLayout* mColorLayout{nullptr};
        QLabel* mColorLabel{nullptr};
        UBColorSwatches* mPageColors{nullptr};

        // The tabs: the patterns of the page, or the templates.
        UBSegmentedChoice* mTabs{nullptr};
        QWidget* mPatterns{nullptr};
        QWidget* mTemplates{nullptr};
        QDateEdit* mTemplateDate{nullptr};
        QCheckBox* mSundayFirst{nullptr};
        QMap<UBPageTemplates::Template, QToolButton*> mTemplateButtons;

        UBSegmentedChoice* mPageFlow{nullptr};
        UBColorSwatches* mLineColors{nullptr};
        QSlider* mLineOpacity{nullptr};
        QLabel* mLineColorLabel{nullptr};
        QLabel* mLineOpacityLabel{nullptr};

        QSlider* mSlider;
        QLabel* mSliderLabel;
        QLabel* mIntermediateLinesLabel;
        UBActionPaletteButton* mResetDefaultGridSizeButton;
        UBActionPaletteButton* mDrawIntermediateLinesCheckBox;

};

#endif // UBBACKGROUNDPALETTE_H
