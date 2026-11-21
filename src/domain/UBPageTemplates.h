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

#include <QByteArray>
#include <QDate>
#include <QList>
#include <QRectF>
#include <QSizeF>
#include <QString>

class QPainter;

/**
 * @brief The page templates of Master Board, like those of note-taking
 * applications: lists, planners, calendars, notes and class sheets.
 *
 * Each template is drawn by Master Board itself on the size of the page,
 * in neutral greys over a transparent background, so that it shows on any
 * page color. The calendars and planners are built for the date given:
 * the days fall on their real weekdays, every year. Their words are in
 * English, Spanish, German, French, Chinese or Japanese; any other
 * language gets the English ones. Their weeks start on Monday, or on
 * Sunday when asked.
 */
class UBPageTemplates
{
public:
    enum class Template
    {
        TodoList,
        BulletList,
        DailyPlanner,
        WeeklyCalendar,
        WeeklyHours,
        MonthlyCalendar,
        YearPlanner,
        ShoppingList,
        HabitTracker,
        CornellNotes,
        Meeting,
        DotGrid,
        Handwriting,
        GraphPaper,
        MusicStaff,
        LessonPlan,
        Attendance,
        Gradebook,
        TwoColumns,
        MindMap,
        Storyboard
    };

    /** @brief All the templates, in the order they are offered. */
    static QList<Template> all();

    /**
     * @brief The words of the templates for this language code of
     * Master Board: "es", "de", "fr", "zh", "ja" or else English.
     */
    explicit UBPageTemplates(const QString& language);

    /** @brief The first day of the week of the calendars: Monday unless Sunday is asked for. */
    void setWeekStartsOnSunday(bool sunday);

    /** @brief The name of the template, in the language of the templates. */
    QString name(Template pageTemplate) const;

    /** @brief Whether the template depends on the date, like the calendars. */
    static bool isDated(Template pageTemplate);

    /** @brief Draws the template in the rectangle, for the date given. */
    void draw(QPainter* painter, const QRectF& rect, Template pageTemplate, const QDate& date) const;

    /** @brief The template as a picture in SVG of the size of the page. */
    QByteArray svg(Template pageTemplate, const QSizeF& pageSize, const QDate& date) const;

    /** @brief Whether this SVG picture is a template made by svg(). */
    static bool isTemplate(const QByteArray& svg);

private:
    QString word(const char* english) const;
    QString monthTitle(const QDate& date) const;
    QString dayName(int dayOfWeek, bool shortName) const;
    QList<QDate> weekOf(const QDate& date) const;

    void drawTitle(QPainter* painter, const QRectF& rect, const QString& title, const QString& right = QString()) const;
    QRectF body(const QRectF& rect) const;
    void drawLines(QPainter* painter, const QRectF& area, qreal spacing, bool withBoxes = false, bool withBullets = false) const;
    void drawGrid(QPainter* painter, const QRectF& area, int columns, int rows) const;
    void drawLabel(QPainter* painter, const QRectF& area, const QString& text, int flags, qreal size, bool bold = false) const;
    void drawMonth(QPainter* painter, const QRectF& area, const QDate& month, bool compact) const;

    void drawDailyPlanner(QPainter* painter, const QRectF& rect, const QDate& date) const;
    void drawWeek(QPainter* painter, const QRectF& rect, const QDate& date, bool withHours) const;
    void drawYear(QPainter* painter, const QRectF& rect, const QDate& date) const;
    void drawHabits(QPainter* painter, const QRectF& rect, const QDate& date) const;
    void drawCornell(QPainter* painter, const QRectF& rect) const;
    void drawSections(QPainter* painter, const QRectF& rect, const QString& title, const QStringList& fields,
                      const QStringList& sections) const;
    void drawTable(QPainter* painter, const QRectF& rect, const QString& title, const QStringList& headers,
                   const QList<qreal>& widths, int rows) const;
    void drawDots(QPainter* painter, const QRectF& rect) const;
    void drawHandwriting(QPainter* painter, const QRectF& rect) const;
    void drawGraphPaper(QPainter* painter, const QRectF& rect) const;
    void drawStaves(QPainter* painter, const QRectF& rect) const;
    void drawMindMap(QPainter* painter, const QRectF& rect) const;
    void drawStoryboard(QPainter* painter, const QRectF& rect) const;

    QString mLanguage;
    Qt::DayOfWeek mFirstDay{Qt::Monday};
};
