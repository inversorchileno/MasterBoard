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



#ifndef UBBOARDVIEW_H_
#define UBBOARDVIEW_H_

#define CONTROLVIEW_OBJ_NAME "ControlView"

#include <QtGui>
#include <QElapsedTimer>
#include <QGraphicsView>
#include <QRubberBand>

#include <optional>

#include "core/UB.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "pdf/UBPdfNavigation.h"

class UBBoardController;
class QGraphicsPathItem;
class UBGraphicsPDFItem;
class UBGraphicsScene;
class UBGraphicsWidgetItem;
class UBRubberBand;
class UBSnapIndicator;

class UBBoardView : public QGraphicsView
{
    Q_OBJECT

public:

    UBBoardView(UBBoardController* pController, QWidget* pParent = 0, bool isControl = false, bool isDesktop = false);
    UBBoardView(UBBoardController* pController, int pStartLayer, int pEndLayer, QWidget* pParent = 0, bool isControl = false, bool isDesktop = false);
    virtual ~UBBoardView();

    std::shared_ptr<UBGraphicsScene> scene();

    void forcedTabletRelease();

    void setToolCursor(int tool);

    void rubberItems();
    void moveRubberedItems(QPointF movingVector);

    void setMultiselection(bool enable);
    bool isMultipleSelectionEnabled() { return mMultipleSelectionIsEnabled; }

    void setBoxing(const QMargins& margins);

    /**
     * @brief Where the view is in the document, in pages from its start:
     * 2.5 is the middle of the third page at the top of the view.
     */
    qreal documentPosition();

    /** @brief The height of the view, in pages. */
    qreal visiblePageFraction();

    /** @brief Shows the page and the place of a document position, at the same zoom. */
    void showDocumentPosition(qreal position);

    /** @brief How the pages follow each other on the board, as in the page settings of Samsung Notes. */
    enum PageFlow
    {
        VerticalFlow = 0,   ///< One page under the other.
        HorizontalFlow = 1, ///< One page beside the other.
        TwoPagesFlow = 2    ///< Two pages side by side, as an open notebook: 1 and 2, 3 and 4...
    };

    static PageFlow pageFlow();

    /** @brief Keeps the way the pages follow each other and shows the pages that way. */
    void setPageFlow(PageFlow flow);

    /** @brief A dark frame telling that this side of the double board is the active one. */
    void setActiveFrameShown(bool shown);
    void updateSnapIndicator(Qt::Corner corner, QPointF snapPoint, double angle = 0);

    // work around for handling tablet events on MAC OS with Qt 4.8.0 and above
#if defined(Q_OS_OSX)
    bool directTabletEvent(QEvent *event);
    QWidget *widgetForTabletEvent(QWidget *w, const QPoint &pos);
#endif
signals:
    void resized(QResizeEvent* event);
    void shown();
    void mouseReleased();
    void painted(const QRectF region);

    /** @brief The view moved in the document: scrolled, zoomed or on another page. */
    void documentPositionChanged();

    /** @brief The pages follow each other another way: under or beside. */
    void pageFlowChanged();

protected:

    bool itemIsLocked(QGraphicsItem *item);
    bool isUBItem(QGraphicsItem *item); // we should to determine items who is not UB and use general scene behavior for them.
    bool isCppTool(QGraphicsItem *item);
    void handleItemsSelection(QGraphicsItem *item);
    bool itemShouldReceiveMousePressEvent(QGraphicsItem *item);
    bool itemShouldReceiveSuspendedMousePressEvent(QGraphicsItem *item);
    bool itemHaveParentWithType(QGraphicsItem *item, int type);
    bool itemShouldBeMoved(QGraphicsItem *item);
    QGraphicsItem* determineItemToPress(QGraphicsItem *item);
    QGraphicsItem* determineItemToMove(QGraphicsItem *item);
    void handleItemMousePress(QMouseEvent *event);
    void handleItemMouseMove(QMouseEvent *event);

    virtual bool event (QEvent * e);

    /**
     * @brief Fingers on a touch screen: one finger works as the mouse, two
     * fingers zoom and move the page. A palm, or a touch while the pen
     * writes, is left out.
     */
    bool viewportEvent(QEvent* event) override;

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void tabletEvent(QTabletEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void leaveEvent ( QEvent * event);

    virtual void focusOutEvent ( QFocusEvent * event );

    virtual void drawItems(QPainter *painter, int numItems,
                           QGraphicsItem *items[],
                           const QStyleOptionGraphicsItem options[]);

    virtual void dropEvent(QDropEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);

    virtual void resizeEvent(QResizeEvent * event);

    virtual void paintEvent(QPaintEvent *event);

    virtual void drawBackground(QPainter *painter, const QRectF &rect);
    virtual void drawForeground(QPainter *painter, const QRectF &rect);

    virtual void scrollContentsBy(int dx, int dy);

private:
        /**
         * @brief Scrolls on past the bottom or the top of the page into the
         * next or previous page, drawn beside it, so that a document is read
         * as one long strip, as in a word processor. Returns true when the
         * wheel event was used for that.
         */
        bool turnPageWithWheel(QWheelEvent* wheelEvent);

        /** @brief Shows the neighbour page as the current one once it fills the middle of the view, without moving the picture. */
        void switchToNeighbourPage();

        /** @brief Draws the pages above and below the current one, as many as the view shows. */
        void drawNeighbourPages(QPainter* painter, const QRectF& rect);

        /** @brief Shows another page at the given zoom, as set by the transform of the view. */
        void showPageAtScale(int index, qreal scale);

        /**
         * @brief Copies the words of the PDF page inside a rectangle of the
         * view, drawn with the selector where there was nothing to select.
         */
        void copyPdfText(const QRect& band);

        /**
         * @brief The PDF page under a point of the view, and the point in
         * points of that page as it is shown. False where there is none.
         */
        bool pdfPointAt(const QPoint& viewPoint, UBGraphicsPDFItem** item, QPointF* onPage);

        /** @brief The link of the PDF under a point of the view, or none. */
        std::optional<UBPdfNavigation::Link> pdfLinkAt(const QPoint& viewPoint);

        /** @brief Takes away the line of the lasso once the selection is done. */
        void removeLasso();

        /** @brief Goes to the page, or opens the address, of a link of the PDF. */
        void followPdfLink(const UBPdfNavigation::Link& link);

        /** @brief The space between two pages, in units of the page. */
        qreal pageGap() const;

        /** @brief The space after a page, before the next one: smaller between the two pages of a pair. */
        qreal gapAfter(int index) const;

        /** @brief True when the pages are beside each other rather than one under the other. */
        static bool flowsAcross();

        /** @brief A page around the current one, and where it is shown, in units of the current page. */
        struct NeighbourPlace
        {
            int index;
            std::shared_ptr<UBGraphicsScene> scene;
            QRectF source;
            QRectF target;
        };

        /** @brief The pages before and after the current one, up to those beyond the given area. */
        QList<NeighbourPlace> neighbourPlaces(const QRectF& area);

        /** @brief Shows a page around the current one as the current page, without moving the picture. */
        void showNeighbourInPlace(int index, const QRectF& target);

        /** @brief Makes the page under a point of the view the current one, when it is one around it. */
        void switchToPageAt(const QPointF& viewPoint);

        /** @brief The two pages of the pair of the current page, in units of the current page. */
        QRectF pairRect();

        /** @brief Shows the pair of pages starting with the given one, at the same zoom. */
        void showPair(int first, bool fromTop);

        /** @brief Zooms so that the two pages of the pair fill the view. */
        void fitPair();

        bool mNeighbourRepaintPending{false};
        bool mShowingInPlace{false};
        bool mActiveFrameShown{false};
        int mLastNeighbourIndex{-1};
        int mNeighbourRepaints{0};
        int mWheelTurn{0};
        bool mSelectorOnNothing{false};

        enum class TouchMode { None, OneFinger, TwoFingers, Ignored };
        bool handleTouch(QTouchEvent* event);
        void sendFingerAsMouse(QEvent::Type type, const QPointF& position);
        TouchMode mTouchMode{TouchMode::None};
        QElapsedTimer mOneFingerTime;
        QElapsedTimer mPenTime;
        QPointF mTouchCenter;
        qreal mTouchSpan{0};
        QPolygon mLassoPoints;
        QGraphicsPathItem* mLassoItem{nullptr};
        bool mOverPdfLink{false};
        QString mPdfLinksOf;
        QList<UBPdfNavigation::Link> mPdfLinks;

private:

    void init();

    inline bool shouldDisplayItem(QGraphicsItem *item)
    {
        bool ok;
        int itemLayerType = item->data(UBGraphicsItemData::ItemLayerType).toInt(&ok);
        return (ok && (itemLayerType >= mStartLayer && itemLayerType <= mEndLayer));
    }

    QList<QUrl> processMimeData(const QMimeData* pMimeData);

    UBBoardController* mController;

    int mStartLayer, mEndLayer;
    bool mFilterZIndex;

    bool mTabletStylusIsPressed;
    bool mUsingTabletEraser;

    bool mPendingStylusReleaseEvent;

    bool mMouseButtonIsPressed;
    QPointF mPreviousPoint;
    QPoint mMouseDownPos;

    bool mPenPressureSensitive;
    bool mMarkerPressureSensitive;
    bool mUseHighResTabletEvent;

    QRubberBand *mRubberBand;
    bool mIsCreatingTextZone;
    bool mIsCreatingSceneGrabZone;
    bool mIsSelectingTextToRead{false};

    bool isAbsurdPoint(QPoint point);

    bool mVirtualKeyboardActive;
    bool mOkOnWidget;

    bool mWidgetMoved;
    QPointF mFirstPressedMousePos;
    QPointF mLastPressedMousePos;
    QList<QPointF> mCornerPoints;

    /* when an item is moved around, the tracking must stop if the object is deleted */
    QGraphicsItem *_movingItem;

    QGraphicsItem *getMovingItem() {
        return _movingItem;
    }

    void setMovingItem(QGraphicsItem *item) {
        // if the current moving item is a qobject, it MUST be disconnected
        if (_movingItem) {
            QObject *moving_item_obj = dynamic_cast<QObject*>(_movingItem);
            if (moving_item_obj != nullptr)
                disconnect(moving_item_obj, &QObject::destroyed, this, &UBBoardView::movingItemDestroyed);
        }

        // attach the new moving item if relevant
        if (item) {
            QObject *item_obj = dynamic_cast<QObject*>(item);
            if (item_obj != nullptr)
                connect(item_obj, &QObject::destroyed, this, &UBBoardView::movingItemDestroyed);
        }
        _movingItem = item;
    }

    QMouseEvent *suspendedMousePressEvent;

    bool moveRubberBand;
    UBRubberBand *mUBRubberBand;

    QList<QGraphicsItem *> mRubberedItems;
    QSet<QGraphicsItem*> mJustSelectedItems;

    int mLongPressInterval;
    QTimer mLongPressTimer;

    bool mIsDragInProgress;
    bool mMultipleSelectionIsEnabled;
    bool bIsControl;
    bool bIsDesktop;
    bool mRubberBandInPlayMode;

    QMargins mMargins{};
    UBSnapIndicator* mSnapIndicator{nullptr};

    static bool hasSelectedParents(QGraphicsItem * item);

private slots:
    void settingChanged(QVariant newValue);
    void movingItemDestroyed(QObject* item = nullptr);

public slots:
    void virtualKeyboardActivated(bool b);
    void longPressEvent();

};

#endif /* UBBOARDVIEW_H_ */
