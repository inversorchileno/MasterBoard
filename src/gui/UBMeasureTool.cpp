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



#include "UBMeasureTool.h"

#include <QApplication>
#include <QButtonGroup>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtMath>

#include <functional>

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "desktop/UBDesktopAnnotationController.h"

namespace
{
    /** @brief The unit and how many points of the page, or of the screen, make one. */
    struct Reference
    {
        QString unit;
        qreal points{0};

        bool isValid() const { return points > 0; }
    };

    Reference sBoardReference;
    Reference sDesktopReference;

    Reference& reference(bool desktop)
    {
        return desktop ? sDesktopReference : sBoardReference;
    }

    /** @brief Where the layer and the window go: the page shown, or the layer over the desktop. */
    QWidget* host(bool desktop)
    {
        if (desktop)
        {
            UBDesktopAnnotationController* controller = UBApplication::applicationController->uninotesController();
            return controller ? controller->drawingView() : nullptr;
        }

        UBBoardView* view = UBApplication::boardController->controlView();
        return view ? view->viewport() : nullptr;
    }

    QGraphicsView* boardView(bool desktop)
    {
        return desktop ? nullptr : UBApplication::boardController->controlView();
    }

    void refreshDesktopMask(bool desktop)
    {
        if (desktop && UBApplication::applicationController->uninotesController())
        {
            QMetaObject::invokeMethod(UBApplication::applicationController->uninotesController(), "refreshMask");
        }
    }

    /** @brief The window of the measure, moved by dragging it. */
    class MeasureWindow : public QFrame
    {
    public:
        explicit MeasureWindow(QWidget* parent)
            : QFrame(parent)
        {
            setObjectName("measureWindow");
            setStyleSheet("QFrame#measureWindow { background: #20242b; border: 1px solid #5a6170; border-radius: 8px; }"
                          "QLabel { color: #e8e8e8; background: transparent; }"
                          "QLabel#measureTitle { color: #9aa3b2; font-size: 12px; }"
                          "QLabel#measureValue { color: #ffffff; font-size: 22px; font-weight: bold; }"
                          "QLabel#measureHint { color: #f6d77a; font-size: 12px; }"
                          "QPushButton { color: #e8e8e8; background: #3a404b; border: 1px solid #5a6170; border-radius: 4px; padding: 3px 12px; }"
                          "QPushButton:hover { background: #4a5260; }"
                          "QToolButton { color: #e8e8e8; background: transparent; border: none; font-size: 15px; padding: 0 4px; }"
                          "QToolButton:hover { background: #c4314b; border-radius: 4px; }"
                          "QRadioButton { color: #e8e8e8; }"
                          "QRadioButton::indicator { width: 12px; height: 12px; border: 1px solid #9aa3b2; border-radius: 7px; background: #111317; }"
                          "QRadioButton::indicator:checked { background: qradialgradient(cx: 0.5, cy: 0.5, radius: 0.5, fx: 0.5, fy: 0.5,"
                          " stop: 0 #f6d77a, stop: 0.5 #f6d77a, stop: 0.6 #111317, stop: 1 #111317); border-color: #f6d77a; }");
            setCursor(Qt::SizeAllCursor);

            auto layout = new QVBoxLayout(this);
            layout->setContentsMargins(12, 6, 6, 10);
            layout->setSpacing(4);

            auto top = new QHBoxLayout();
            mTitle = new QLabel(this);
            mTitle->setObjectName("measureTitle");
            mClose = new QToolButton(this);
            mClose->setText(QStringLiteral("✕"));
            mClose->setToolTip(UBMeasureTool::tr("End the measurements"));
            mClose->setCursor(Qt::ArrowCursor);
            top->addWidget(mTitle, 1);
            top->addWidget(mClose);
            layout->addLayout(top);

            mValue = new QLabel(this);
            mValue->setObjectName("measureValue");
            layout->addWidget(mValue);

            mHint = new QLabel(this);
            mHint->setObjectName("measureHint");
            mHint->setWordWrap(true);
            layout->addWidget(mHint);

            // The choice of the unit, in the window itself: a window of its
            // own would not get the clicks over the desktop.
            mUnits = new QWidget(this);
            auto unitsLayout = new QVBoxLayout(mUnits);
            unitsLayout->setContentsMargins(0, 2, 0, 0);
            auto unitsHelp = new QLabel(UBMeasureTool::tr("Choose the unit. Then draw a line that measures 1 of that unit: "
                                                          "the measures are made with that reference."), mUnits);
            unitsHelp->setObjectName("measureHint");
            unitsHelp->setWordWrap(true);
            unitsLayout->addWidget(unitsHelp);

            mUnitChoice = new QButtonGroup(mUnits);
            auto columns = new QHBoxLayout();
            const QList<QPair<QString, QStringList>> systems = {
                { UBMeasureTool::tr("Metric"), { "mm", "cm", "m", "km" } },
                { UBMeasureTool::tr("Imperial"), { "in", "ft", "yd", "mi" } },
            };
            const QMap<QString, QString> names = {
                { "mm", UBMeasureTool::tr("millimeter (mm)") }, { "cm", UBMeasureTool::tr("centimeter (cm)") },
                { "m", UBMeasureTool::tr("meter (m)") }, { "km", UBMeasureTool::tr("kilometer (km)") },
                { "in", UBMeasureTool::tr("inch (in)") }, { "ft", UBMeasureTool::tr("foot (ft)") },
                { "yd", UBMeasureTool::tr("yard (yd)") }, { "mi", UBMeasureTool::tr("mile (mi)") },
            };

            for (const auto& system : systems)
            {
                auto column = new QVBoxLayout();
                auto name = new QLabel(system.first, mUnits);
                name->setObjectName("measureTitle");
                column->addWidget(name);

                for (const QString& unit : system.second)
                {
                    auto choice = new QRadioButton(names.value(unit), mUnits);
                    choice->setProperty("unit", unit);
                    choice->setCursor(Qt::ArrowCursor);
                    mUnitChoice->addButton(choice);
                    column->addWidget(choice);
                }

                columns->addLayout(column);
            }

            unitsLayout->addLayout(columns);
            mUnitDone = new QPushButton(UBMeasureTool::tr("Continue"), mUnits);
            mUnitDone->setCursor(Qt::ArrowCursor);
            auto doneRow = new QHBoxLayout();
            doneRow->addStretch(1);
            doneRow->addWidget(mUnitDone);
            unitsLayout->addLayout(doneRow);
            mUnits->hide();
            layout->addWidget(mUnits);

            connect(mUnitDone, &QPushButton::clicked, this, [this]() {
                if (mUnitChoice->checkedButton() && mUnitChosen)
                {
                    mUnits->hide();
                    mRestart->show();
                    mUnitChosen(mUnitChoice->checkedButton()->property("unit").toString());
                }
            });

            auto bottom = new QHBoxLayout();
            mFinish = new QPushButton(UBMeasureTool::tr("Finish measure"), this);
            mFinish->setToolTip(UBMeasureTool::tr("Ends the shape at its last corner, as Enter does"));
            mFinish->setCursor(Qt::ArrowCursor);
            mFinish->hide();
            mModify = new QPushButton(UBMeasureTool::tr("Modify"), this);
            mModify->setToolTip(UBMeasureTool::tr("Move the corners of the shape; Enter or Finish measure ends"));
            mModify->setCursor(Qt::ArrowCursor);
            mModify->hide();
            mRestart = new QPushButton(UBMeasureTool::tr("Restart"), this);
            mRestart->setToolTip(UBMeasureTool::tr("Choose the unit and draw the reference again"));
            mRestart->setCursor(Qt::ArrowCursor);
            bottom->addWidget(mFinish);
            bottom->addStretch(1);
            bottom->addWidget(mModify);
            bottom->addWidget(mRestart);
            layout->addLayout(bottom);

            setFixedWidth(300);
        }

        void show(const QString& title, const QString& value, const QString& hint)
        {
            const bool choosing = mUnits->isVisible();
            mTitle->setText(title);
            mValue->setText(value);
            mValue->setVisible(!value.isEmpty() && !choosing);
            mHint->setText(hint);
            mHint->setVisible(!hint.isEmpty() && !choosing);
            adjustSize();
            QFrame::show();
            raise();
        }

        /** @brief Asks for the unit; the function gets it. */
        void askUnit(const QString& current, std::function<void(const QString&)> chosen)
        {
            mUnitChosen = std::move(chosen);

            for (QAbstractButton* button : mUnitChoice->buttons())
            {
                button->setChecked(button->property("unit").toString() == current);
            }

            mValue->hide();
            mHint->hide();
            mRestart->hide();
            mFinish->hide();
            mModify->hide();
            mUnits->show();
            adjustSize();
            QFrame::show();
            raise();
        }

        bool isChoosingUnit() const
        {
            return mUnits->isVisible();
        }

        /** @brief Shows Finish measure and Modify when they apply. */
        void setActions(bool finish, bool modify)
        {
            mFinish->setVisible(finish && !mUnits->isVisible());
            mModify->setVisible(modify && !mUnits->isVisible());
        }

        QToolButton* mClose{nullptr};
        QPushButton* mRestart{nullptr};
        QPushButton* mFinish{nullptr};
        QPushButton* mModify{nullptr};

    protected:
        void mousePressEvent(QMouseEvent* event) override
        {
            mGrab = event->pos();
        }

        void mouseMoveEvent(QMouseEvent* event) override
        {
            if (event->buttons() & Qt::LeftButton)
            {
                QPoint position = mapToParent(event->pos() - mGrab);
                position.setX(qBound(0, position.x(), parentWidget()->width() - width()));
                position.setY(qBound(0, position.y(), parentWidget()->height() - height()));
                move(position);
            }
        }

        void mouseReleaseEvent(QMouseEvent*) override
        {
            refreshDesktopMask(UBApplication::applicationController->isShowingDesktop());
        }

    private:
        QLabel* mTitle{nullptr};
        QLabel* mValue{nullptr};
        QLabel* mHint{nullptr};
        QWidget* mUnits{nullptr};
        QButtonGroup* mUnitChoice{nullptr};
        QPushButton* mUnitDone{nullptr};
        std::function<void(const QString&)> mUnitChosen;
        QPoint mGrab;
    };

    QPointer<MeasureWindow> sWindow;

    /**
     * @brief The layer over the page or the desktop where the reference and
     * the measures are drawn, with the points in the coordinates of the page
     * on the board.
     */
    class MeasureLayer : public QWidget
    {
    public:
        MeasureLayer(QWidget* parent, bool desktop, UBMeasureTool::Mode mode)
            : QWidget(parent)
            , mDesktop(desktop)
            , mMode(mode)
        {
            setAttribute(Qt::WA_TranslucentBackground);
            setMouseTracking(true);
            setCursor(Qt::CrossCursor);
            setFocusPolicy(Qt::StrongFocus);
            setGeometry(parent->rect());

            // The page may scroll or zoom under the lines.
            auto repaint = new QTimer(this);
            connect(repaint, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
            repaint->start(80);

            show();

            // On the desktop, under the palette and the windows of the layer
            // over the desktop, above its drawings only.
            if (auto view = qobject_cast<QGraphicsView*>(parent))
            {
                lower();
                view->viewport()->lower();
            }
            else
            {
                raise();
            }

            setFocus();
            showMeasure();
        }

        /** @brief The next line drawn is the reference of one unit. */
        void setCalibrating(bool calibrating)
        {
            mCalibrating = calibrating;
            mEditing = false;
            mPoints.clear();
            mFinished = false;
            showMeasure();
        }

        void setMode(UBMeasureTool::Mode mode)
        {
            mMode = mode;
            mEditing = false;
            mPoints.clear();
            mFinished = false;
            showMeasure();
        }

    protected:
        QPointF toPoint(const QPointF& position) const
        {
            QGraphicsView* view = boardView(mDesktop);
            return view ? view->mapToScene(position.toPoint()) : position;
        }

        QPointF toScreen(const QPointF& point) const
        {
            QGraphicsView* view = boardView(mDesktop);
            return view ? QPointF(view->mapFromScene(point)) : point;
        }

        bool isLine() const
        {
            return mCalibrating || mMode == UBMeasureTool::Mode::Distance;
        }

        void mousePressEvent(QMouseEvent* event) override
        {
            if (event->button() != Qt::LeftButton)
            {
                event->ignore();
                return;
            }

            // Nothing to measure with before the unit and its reference.
            if (!mCalibrating && !reference(mDesktop).isValid())
            {
                return;
            }

            const QPointF point = toPoint(event->position());

            if (mEditing)
            {
                // The corner under the pointer follows it until released.
                mGrabbed = -1;

                for (int i = 0; i < mPoints.size(); ++i)
                {
                    if (QLineF(toScreen(mPoints.at(i)), event->position()).length() <= 12)
                    {
                        mGrabbed = i;
                    }
                }

                return;
            }

            if (isLine())
            {
                // A line: a click at its start, a click at its end.
                if (mPoints.isEmpty() || mFinished)
                {
                    mPoints = { point, point };
                    mFinished = false;
                }
                else
                {
                    mPoints.last() = point;
                    finishLine();
                }
            }
            else
            {
                // A shape: a click at each corner, a double click at the last one.
                if (mFinished)
                {
                    mPoints.clear();
                    mFinished = false;
                }

                if (mPoints.isEmpty())
                {
                    mPoints << point;
                }

                mPoints << point;
            }

            showMeasure();
            update();
        }

        void mouseMoveEvent(QMouseEvent* event) override
        {
            if (mEditing)
            {
                if (mGrabbed >= 0 && (event->buttons() & Qt::LeftButton))
                {
                    mPoints[mGrabbed] = toPoint(event->position());
                    showMeasure();
                    update();
                }

                return;
            }

            if (!mPoints.isEmpty() && !mFinished)
            {
                mPoints.last() = toPoint(event->position());
                showMeasure();
                update();
            }
        }

        void mouseReleaseEvent(QMouseEvent*) override
        {
            mGrabbed = -1;
        }

        void mouseDoubleClickEvent(QMouseEvent* event) override
        {
            if (mEditing || isLine())
            {
                mousePressEvent(event);
                return;
            }

            // The last corner: the double click added it twice.
            if (mPoints.size() > 2)
            {
                mPoints.removeLast();
            }

            mFinished = mPoints.size() >= 2;
            showMeasure();
            update();
        }

        void keyPressEvent(QKeyEvent* event) override
        {
            if (event->key() == Qt::Key_Escape && !mEditing)
            {
                mPoints.clear();
                mFinished = false;
                showMeasure();
                update();
                return;
            }

            if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            {
                finish();
                return;
            }

            QWidget::keyPressEvent(event);
        }

    public:
        /** @brief Ends the shape, as Enter and Finish measure do, or the changes of its corners. */
        void finish()
        {
            if (mEditing)
            {
                mEditing = false;
                setCursor(Qt::CrossCursor);
            }
            else if (!isLine() && !mFinished && mPoints.size() > 2)
            {
                // Without the corner that follows the pointer.
                mPoints.removeLast();
                mFinished = true;
            }

            showMeasure();
            update();
        }

        /** @brief The corners of the finished shape can be moved. */
        void startEditing()
        {
            if (!isLine() && mFinished && mPoints.size() > 1)
            {
                mEditing = true;
                setCursor(Qt::PointingHandCursor);
                setFocus();
                showMeasure();
                update();
            }
        }

    protected:
        void finishLine()
        {
            mFinished = true;

            if (!mCalibrating)
            {
                return;
            }

            // A reference too short to be meant is drawn again.
            const qreal length = QLineF(mPoints.first(), mPoints.last()).length();

            if (QLineF(toScreen(mPoints.first()), toScreen(mPoints.last())).length() >= 8)
            {
                reference(mDesktop).points = length;
                mCalibrating = false;
            }

            mPoints.clear();
            mFinished = false;
        }

        void wheelEvent(QWheelEvent* event) override
        {
            // The page scrolls and zooms under the measures.
            event->ignore();
        }

        void paintEvent(QPaintEvent*) override
        {
            if (mPoints.isEmpty())
            {
                return;
            }

            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            QPolygonF shape;

            for (const QPointF& point : std::as_const(mPoints))
            {
                shape << toScreen(point);
            }

            const QColor red(229, 72, 77);
            const bool closed = !mCalibrating && mMode != UBMeasureTool::Mode::Distance && shape.size() > 2;

            if (!mCalibrating && mMode == UBMeasureTool::Mode::Area && shape.size() > 2)
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(229, 72, 77, 60));
                painter.drawPolygon(shape);
            }

            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(mCalibrating ? QColor(47, 95, 168) : red, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

            if (closed)
            {
                painter.drawPolygon(shape);
            }
            else
            {
                painter.drawPolyline(shape);
            }

            painter.setPen(QPen(Qt::white, 1.5));
            painter.setBrush(mCalibrating ? QColor(47, 95, 168) : red);

            for (const QPointF& corner : std::as_const(shape))
            {
                painter.drawEllipse(corner, 4, 4);
            }

            // The measure beside the last point.
            const QString text = valueText();

            if (!text.isEmpty())
            {
                QFont font = painter.font();
                font.setPixelSize(15);
                font.setBold(true);
                painter.setFont(font);
                const QRectF box = QRectF(shape.last() + QPointF(12, 12), QSizeF(painter.fontMetrics().horizontalAdvance(text) + 14, 24));
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(32, 36, 43, 220));
                painter.drawRoundedRect(box, 5, 5);
                painter.setPen(Qt::white);
                painter.drawText(box, Qt::AlignCenter, text);
            }
        }

    private:
        /** @brief The measure of what is drawn, in the unit of the reference. */
        QString valueText() const
        {
            const Reference& unit = reference(mDesktop);

            if (mCalibrating || !unit.isValid() || mPoints.size() < 2)
            {
                return QString();
            }

            const QLocale locale;
            qreal length = 0;

            for (int i = 1; i < mPoints.size(); ++i)
            {
                length += QLineF(mPoints.at(i - 1), mPoints.at(i)).length();
            }

            if (mMode == UBMeasureTool::Mode::Distance)
            {
                return QString("%1 %2").arg(locale.toString(length / unit.points, 'f', 2), unit.unit);
            }

            if (mMode == UBMeasureTool::Mode::Perimeter)
            {
                if (mPoints.size() > 2)
                {
                    length += QLineF(mPoints.last(), mPoints.first()).length();
                }

                return QString("%1 %2").arg(locale.toString(length / unit.points, 'f', 2), unit.unit);
            }

            // The area of the polygon, by the shoelace formula.
            qreal area = 0;

            for (int i = 0; i < mPoints.size(); ++i)
            {
                const QPointF& a = mPoints.at(i);
                const QPointF& b = mPoints.at((i + 1) % mPoints.size());
                area += a.x() * b.y() - b.x() * a.y();
            }

            area = qAbs(area) / 2 / (unit.points * unit.points);
            return QString("%1 %2²").arg(locale.toString(area, 'f', 2), unit.unit);
        }

        void showMeasure()
        {
            if (!sWindow || sWindow->isChoosingUnit())
            {
                return;
            }

            const Reference& unit = reference(mDesktop);
            QString title;
            QString hint;

            switch (mMode)
            {
            case UBMeasureTool::Mode::Distance:
                title = UBMeasureTool::tr("Distance");
                hint = UBMeasureTool::tr("Click at the start and at the end.");
                break;
            case UBMeasureTool::Mode::Perimeter:
                title = UBMeasureTool::tr("Perimeter");
                hint = UBMeasureTool::tr("Click at each corner; double click at the last one.");
                break;
            default:
                title = UBMeasureTool::tr("Area");
                hint = UBMeasureTool::tr("Click at each corner; double click at the last one.");
                break;
            }

            if (mCalibrating)
            {
                hint = UBMeasureTool::tr("Click at the start and at the end of a line that measures 1 %1: "
                                         "it is the reference of the measures.").arg(unit.unit);
                sWindow->show(title, QString(), hint);
                sWindow->setActions(false, false);
                return;
            }

            QString value = valueText();

            if (value.isEmpty())
            {
                value = QString("— %1").arg(mMode == UBMeasureTool::Mode::Area ? unit.unit + "²" : unit.unit);
            }

            if (mEditing)
            {
                hint = UBMeasureTool::tr("Drag the corners; Enter or Finish measure ends.");
            }

            sWindow->show(title, value, mFinished && !mEditing ? QString() : hint);

            // Finish measure while a shape is drawn or changed, Modify once it is done.
            const bool shape = !isLine();
            sWindow->setActions(shape && (mEditing || (!mFinished && mPoints.size() > 2)),
                                shape && mFinished && !mEditing);
        }

        bool mDesktop{false};
        UBMeasureTool::Mode mMode{UBMeasureTool::Mode::Distance};
        bool mCalibrating{false};
        bool mFinished{false};
        bool mEditing{false};
        int mGrabbed{-1};
        QList<QPointF> mPoints;
    };

    QPointer<MeasureLayer> sLayer;
}

UBMeasureTool* UBMeasureTool::instance()
{
    static UBMeasureTool* tool = new UBMeasureTool(qApp);
    return tool;
}

UBMeasureTool::UBMeasureTool(QObject* parent)
    : QObject(parent)
{
    // Another tool: the window keeps the last measure, the lines go until
    // the measure tool is chosen again.
    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged, this, &UBMeasureTool::toolChanged);

    // From the board to the desktop, or back: the measurements end.
    connect(UBApplication::applicationController, &UBApplicationController::desktopMode, this, [this](bool) { close(); });
}

void UBMeasureTool::choose(Mode mode)
{
    if (mode == mMode)
    {
        close();
        return;
    }

    const bool desktop = UBApplication::applicationController->isShowingDesktop();

    if (mMode != Mode::None && desktop != mDesktop)
    {
        close();
    }

    mDesktop = desktop;
    QWidget* parent = host(mDesktop);

    if (!parent)
    {
        return;
    }

    mMode = mode;
    emit modeChanged();

    if (!sWindow)
    {
        sWindow = new MeasureWindow(parent);
        // On the board at the top left of the page, away from the toolbar;
        // on the desktop at the top right.
        sWindow->move(mDesktop ? parent->width() - 330 : 20, 20);
        connect(sWindow->mClose, &QToolButton::clicked, this, &UBMeasureTool::close);
        connect(sWindow->mRestart, &QPushButton::clicked, this, &UBMeasureTool::restart);
        connect(sWindow->mFinish, &QPushButton::clicked, this, []() {
            if (sLayer)
            {
                sLayer->finish();
            }
        });
        connect(sWindow->mModify, &QPushButton::clicked, this, []() {
            if (sLayer)
            {
                sLayer->startEditing();
            }
        });
        sWindow->QFrame::show();
    }

    if (UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Measure)
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Measure);
    }

    showLayer();

    // The unit and the reference, the first time.
    if (!reference(mDesktop).isValid())
    {
        askUnit();
    }

    refreshDesktopMask(mDesktop);
}

void UBMeasureTool::showLayer()
{
    QWidget* parent = host(mDesktop);

    if (!parent)
    {
        return;
    }

    if (!sLayer)
    {
        sLayer = new MeasureLayer(parent, mDesktop, mMode);
    }
    else
    {
        sLayer->setMode(mMode);
    }

    if (sWindow)
    {
        sWindow->raise();
    }
}

void UBMeasureTool::restart()
{
    showLayer();
    askUnit();
}

void UBMeasureTool::askUnit()
{
    if (!sWindow)
    {
        return;
    }

    const QString current = reference(mDesktop).unit.isEmpty() ? QString("cm") : reference(mDesktop).unit;

    sWindow->askUnit(current, [this](const QString& unit) {
        Reference& chosen = reference(mDesktop);
        chosen.unit = unit;
        chosen.points = 0;
        showLayer();

        if (sLayer)
        {
            sLayer->setCalibrating(true);
        }

        refreshDesktopMask(mDesktop);
    });

    refreshDesktopMask(mDesktop);
}

void UBMeasureTool::toolChanged(int tool)
{
    if (mMode == Mode::None)
    {
        return;
    }

    if (tool == UBStylusTool::Measure)
    {
        showLayer();
    }
    else if (sLayer)
    {
        // The measure stays in its window, as it was.
        sLayer->deleteLater();
        sLayer.clear();
    }

    refreshDesktopMask(mDesktop);
}

void UBMeasureTool::close()
{
    if (sLayer)
    {
        sLayer->deleteLater();
        sLayer.clear();
    }

    if (sWindow)
    {
        sWindow->deleteLater();
        sWindow.clear();
    }

    const bool wasDesktop = mDesktop;
    mMode = Mode::None;
    emit modeChanged();
    refreshDesktopMask(wasDesktop);
}

QRect UBMeasureTool::desktopArea()
{
    if (sWindow && sWindow->isVisible() && UBApplication::applicationController->isShowingDesktop())
    {
        return sWindow->geometry();
    }

    return QRect();
}
