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


#include "UBPdfAnnotations.h"

#include <QFile>
#include <QFileInfo>
#include <QTransform>

#include <cstring>

// PDFDoc first: it brings the objects of poppler in the order they need.
#include <poppler/PDFDoc.h>
#include <poppler/Array.h>
#include <poppler/Catalog.h>
#include <poppler/Dict.h>
#include <poppler/GfxState.h>
#include <poppler/GlobalParams.h>
#include <poppler/Object.h>
#include <poppler/Page.h>
#include <poppler/Stream.h>
#include <poppler/XRef.h>
#include <poppler/cpp/poppler-version.h>
#include <poppler/goo/gmem.h>

namespace
{
    // A text of the PDF, in UTF-16 with its mark, for any language.
    GooString* pdfText(const QString& text)
    {
        QByteArray bytes("\xFE\xFF", 2);

        for (const QChar c : text)
        {
            bytes.append(char(c.unicode() >> 8));
            bytes.append(char(c.unicode() & 0xFF));
        }

        return new GooString(bytes.constData(), size_t(bytes.size()));
    }

    GooString* pdfDate(const QDateTime& date)
    {
        const QDateTime utc = (date.isValid() ? date : QDateTime::currentDateTime()).toUTC();
        return new GooString(("D:" + utc.toString("yyyyMMddHHmmss") + "Z").toLatin1().constData());
    }

    Object numbers(XRef* xref, std::initializer_list<double> values)
    {
        auto array = new Array(xref);

        for (double value : values)
        {
            array->add(Object(value));
        }

        return Object(array);
    }

    // Recent versions of poppler ask whether to compress the stream; older ones do not.
    template<class X>
    auto addStream(X* xref, Dict* dict, char* buffer, Goffset size, int) -> decltype(xref->addStreamObject(dict, buffer, size, {}))
    {
        return xref->addStreamObject(dict, buffer, size, {});
    }

    template<class X>
    Ref addStream(X* xref, Dict* dict, char* buffer, Goffset size, long)
    {
        return xref->addStreamObject(dict, buffer, size);
    }

    // The file as a stream of the PDF.
    Ref embedFile(XRef* xref, const QString& path)
    {
        QFile file(path);

        if (!file.open(QIODevice::ReadOnly))
        {
            return Ref::INVALID();
        }

        const QByteArray content = file.readAll();
        char* buffer = static_cast<char*>(gmalloc(size_t(qMax<qsizetype>(1, content.size()))));
        memcpy(buffer, content.constData(), size_t(content.size()));

        auto dict = new Dict(xref);
        dict->add("Type", Object(objName, "EmbeddedFile"));

        auto params = new Dict(xref);
        params->add("Size", Object(int(content.size())));
        params->add("ModDate", Object(pdfDate(QFileInfo(path).lastModified())));
        dict->add("Params", Object(params));

        // The stream frees the buffer.
        return addStream(xref, dict, buffer, content.size(), 0);
    }
}

bool UBPdfAnnotations::write(const QString& pdfFile, const QList<Mark>& marks)
{
    if (marks.isEmpty())
    {
        return true;
    }

    if (!globalParams)
    {
        return false;
    }

    // Written beside the file, then put in its place.
    const QString partial = pdfFile + ".notes";

    {
#if POPPLER_VERSION_MAJOR > 25 || (POPPLER_VERSION_MAJOR == 25 && POPPLER_VERSION_MINOR >= 12)
        PDFDoc document(std::make_unique<GooString>(static_cast<std::string_view>(pdfFile.toLocal8Bit())));
#elif POPPLER_VERSION_MAJOR > 22 || (POPPLER_VERSION_MAJOR == 22 && POPPLER_VERSION_MINOR >= 3)
        PDFDoc document(std::make_unique<GooString>(pdfFile.toLocal8Bit()));
#else
        PDFDoc document(new GooString(pdfFile.toLocal8Bit()), nullptr, nullptr, nullptr);
#endif

        if (!document.isOk())
        {
            return false;
        }

        XRef* xref = document.getXRef();
        QMap<int, QList<Ref>> annotationsOfPage;

        for (const Mark& mark : marks)
        {
            Page* page = mark.page >= 1 && mark.page <= document.getNumPages() ? document.getPage(mark.page) : nullptr;

            if (!page)
            {
                continue;
            }

            // From the place on the page as it is shown to the coordinates of the PDF.
            GfxState state(72, 72, page->getMediaBox(), page->getRotate(), true);
            const double* ctm = state.getCTM();
            const QTransform toShown(ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
            const QPointF shown(mark.place.x() * state.getPageWidth(), mark.place.y() * state.getPageHeight());
            const QPointF onPdf = toShown.inverted().map(shown);
            const double half = 10;

            auto annotation = new Dict(xref);
            annotation->add("Type", Object(objName, "Annot"));
            annotation->add("Rect", numbers(xref, { onPdf.x() - half, onPdf.y() - half, onPdf.x() + half, onPdf.y() + half }));
            annotation->add("F", Object(4)); // printed
            annotation->add("M", Object(pdfDate(mark.date)));
            annotation->add("T", Object(pdfText(QStringLiteral("Master Board"))));

            if (mark.file.isEmpty())
            {
                annotation->add("Subtype", Object(objName, "Text"));
                annotation->add("Name", Object(objName, "Comment"));
                annotation->add("Contents", Object(pdfText(mark.text)));
                annotation->add("C", numbers(xref, { 1.0, 0.82, 0.2 }));
                annotation->add("Open", Object(false));
            }
            else
            {
                const Ref stream = embedFile(xref, mark.file);

                if (stream == Ref::INVALID())
                {
                    delete annotation;
                    continue;
                }

                const QString name = QFileInfo(mark.file).fileName();
                auto embedded = new Dict(xref);
                embedded->add("F", Object(stream));

                auto specification = new Dict(xref);
                specification->add("Type", Object(objName, "Filespec"));
                specification->add("F", Object(pdfText(name)));
                specification->add("UF", Object(pdfText(name)));
                specification->add("EF", Object(embedded));

                annotation->add("Subtype", Object(objName, "FileAttachment"));
                annotation->add("Name", Object(objName, "PushPin"));
                annotation->add("Contents", Object(pdfText(name)));
                annotation->add("FS", Object(specification));
                annotation->add("C", numbers(xref, { 0.24, 0.48, 0.85 }));
            }

            annotationsOfPage[mark.page] << xref->addIndirectObject(Object(annotation));
        }

        // Each page lists its annotations, the old ones and the new ones.
        for (auto it = annotationsOfPage.constBegin(); it != annotationsOfPage.constEnd(); ++it)
        {
            Ref* pageRef = document.getCatalog()->getPageRef(it.key());

            if (!pageRef)
            {
                continue;
            }

            Object pageObject = xref->fetch(*pageRef);

            if (!pageObject.isDict())
            {
                continue;
            }

            auto annotations = new Array(xref);
            Object existing = pageObject.dictLookup("Annots");

            if (existing.isArray())
            {
                for (int i = 0; i < existing.arrayGetLength(); ++i)
                {
                    annotations->add(existing.arrayGetNF(i).copy());
                }
            }

            for (const Ref& annotation : it.value())
            {
                annotations->add(Object(annotation));
            }

            pageObject.dictSet("Annots", Object(annotations));
            xref->setModifiedObject(&pageObject, *pageRef);
        }

        QFile::remove(partial);

        if (document.saveAs(GooString(partial.toLocal8Bit().constData())) != errNone)
        {
            QFile::remove(partial);
            return false;
        }
    }

    return QFile::remove(pdfFile) && QFile::rename(partial, pdfFile);
}

namespace
{
    // The width of the letters of Helvetica, in thousandths of the size: close
    // enough to stretch each word over its place.
    qreal letterWidth(QChar c)
    {
        if (c == QLatin1Char(' ')) return 278;
        if (QStringLiteral("ijlI.,:;'!|").contains(c)) return 250;
        if (QStringLiteral("frt()-").contains(c)) return 333;
        if (QStringLiteral("mwMW").contains(c)) return 850;
        if (c.isUpper()) return 690;
        if (c.isDigit()) return 556;
        return 520;
    }

    // A text of the PDF in the encoding of its standard fonts, with its signs escaped.
    QByteArray winAnsi(const QString& text)
    {
        QByteArray bytes;

        for (const QChar c : text)
        {
            const ushort code = c.unicode();
            const char byte = (code >= 32 && code < 256 && !(code >= 127 && code < 160)) ? char(code) : '?';

            if (byte == '(' || byte == ')' || byte == '\\')
            {
                bytes.append('\\');
            }

            bytes.append(byte);
        }

        return bytes;
    }

    // A point of the original page, from 0 to 1, as it is on the page written.
    QPointF placed(QPointF relative, const UBPdfAnnotations::TextPage& page)
    {
        switch (page.turn)
        {
        case 90:
            relative = QPointF(1 - relative.y(), relative.x());
            break;
        case 180:
            relative = QPointF(1 - relative.x(), 1 - relative.y());
            break;
        case 270:
            relative = QPointF(relative.y(), 1 - relative.x());
            break;
        default:
            break;
        }

        return QPointF(page.placement.left() + relative.x() * page.placement.width(),
                       page.placement.top() + relative.y() * page.placement.height());
    }

    QString number(qreal value)
    {
        return QString::number(value, 'f', 3);
    }
}

bool UBPdfAnnotations::writeTextLayer(const QString& pdfFile, const QList<TextPage>& pages)
{
    bool anyWord = false;

    for (const TextPage& page : pages)
    {
        anyWord = anyWord || !page.words.isEmpty();
    }

    if (!anyWord)
    {
        return true;
    }

    if (!globalParams)
    {
        return false;
    }

    const QString partial = pdfFile + ".text";

    {
#if POPPLER_VERSION_MAJOR > 25 || (POPPLER_VERSION_MAJOR == 25 && POPPLER_VERSION_MINOR >= 12)
        PDFDoc document(std::make_unique<GooString>(static_cast<std::string_view>(pdfFile.toLocal8Bit())));
#elif POPPLER_VERSION_MAJOR > 22 || (POPPLER_VERSION_MAJOR == 22 && POPPLER_VERSION_MINOR >= 3)
        PDFDoc document(std::make_unique<GooString>(pdfFile.toLocal8Bit()));
#else
        PDFDoc document(new GooString(pdfFile.toLocal8Bit()), nullptr, nullptr, nullptr);
#endif

        if (!document.isOk())
        {
            return false;
        }

        XRef* xref = document.getXRef();

        // One font for all the pages: the words are invisible, any font reads them.
        auto font = new Dict(xref);
        font->add("Type", Object(objName, "Font"));
        font->add("Subtype", Object(objName, "Type1"));
        font->add("BaseFont", Object(objName, "Helvetica"));
        font->add("Encoding", Object(objName, "WinAnsiEncoding"));
        const Ref fontRef = xref->addIndirectObject(Object(font));

        for (const TextPage& textPage : pages)
        {
            Page* page = textPage.page >= 1 && textPage.page <= document.getNumPages() ? document.getPage(textPage.page) : nullptr;
            Ref* pageRef = page ? document.getCatalog()->getPageRef(textPage.page) : nullptr;

            if (!page || !pageRef || textPage.words.isEmpty() || textPage.sourceSize.isEmpty())
            {
                continue;
            }

            // From the page as it is shown to the coordinates of the PDF.
            GfxState state(72, 72, page->getMediaBox(), page->getRotate(), true);
            const double* ctm = state.getCTM();
            const QTransform toPdf = QTransform(ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]).inverted();
            const QSizeF shown(state.getPageWidth(), state.getPageHeight());

            auto onPdf = [&](QPointF point) {
                const QPointF relative(point.x() / textPage.sourceSize.width(), point.y() / textPage.sourceSize.height());
                const QPointF there = placed(relative, textPage);
                return toPdf.map(QPointF(there.x() * shown.width(), there.y() * shown.height()));
            };

            // Text drawn in the mode that neither fills nor strokes: invisible.
            QString text = QStringLiteral("Q\nBT\n3 Tr\n");

            for (const UBOcrEngine::Word& word : textPage.words)
            {
                const QByteArray encoded = winAnsi(word.text);
                qreal units = 0;

                for (const QChar c : word.text)
                {
                    units += letterWidth(c) / 1000.0;
                }

                if (encoded.isEmpty() || units <= 0)
                {
                    continue;
                }

                // The line under the letters, a fifth of the height above the bottom.
                const QRectF r = word.rect;
                const QPointF bottomLeft = onPdf(QPointF(r.left(), r.bottom() - r.height() * 0.2));
                const QPointF bottomRight = onPdf(QPointF(r.right(), r.bottom() - r.height() * 0.2));
                const QPointF topLeft = onPdf(QPointF(r.left(), r.top()));
                const QPointF across = (bottomRight - bottomLeft) / units;
                const QPointF up = (topLeft - bottomLeft) / 0.8;

                text += QStringLiteral("/LBOcr 1 Tf %1 %2 %3 %4 %5 %6 Tm (")
                            .arg(number(across.x()), number(across.y()), number(up.x()), number(up.y()),
                                 number(bottomLeft.x()), number(bottomLeft.y()));
                text += QString::fromLatin1(encoded) + QStringLiteral(") Tj\n");
            }

            text += QStringLiteral("ET\n");

            // The drawing of the page in its own state, then the words.
            const QByteArray before("q\n");
            const QByteArray after = text.toLatin1();
            char* beforeBuffer = static_cast<char*>(gmalloc(size_t(before.size())));
            memcpy(beforeBuffer, before.constData(), size_t(before.size()));
            char* afterBuffer = static_cast<char*>(gmalloc(size_t(after.size())));
            memcpy(afterBuffer, after.constData(), size_t(after.size()));
            const Ref beforeRef = addStream(xref, new Dict(xref), beforeBuffer, before.size(), 0);
            const Ref afterRef = addStream(xref, new Dict(xref), afterBuffer, after.size(), 0);

            Object pageObject = xref->fetch(*pageRef);

            if (!pageObject.isDict())
            {
                continue;
            }

            auto contents = new Array(xref);
            contents->add(Object(beforeRef));
            const Object& existing = pageObject.dictLookupNF("Contents");

            if (existing.isArray())
            {
                for (int i = 0; i < existing.arrayGetLength(); ++i)
                {
                    contents->add(existing.arrayGetNF(i).copy());
                }
            }
            else if (existing.isRef())
            {
                // A reference may name a list of streams as well as one stream.
                Object resolved = xref->fetch(existing.getRef());

                if (resolved.isArray())
                {
                    for (int i = 0; i < resolved.arrayGetLength(); ++i)
                    {
                        contents->add(resolved.arrayGetNF(i).copy());
                    }
                }
                else
                {
                    contents->add(existing.copy());
                }
            }

            contents->add(Object(afterRef));

            // The resources of the page, inherited ones included, with the font.
            auto resources = new Dict(xref);
            Dict* oldResources = page->getResourceDict();
            auto fonts = new Dict(xref);

            if (oldResources)
            {
                for (int i = 0; i < oldResources->getLength(); ++i)
                {
                    const char* key = oldResources->getKey(i);

                    if (strcmp(key, "Font") == 0)
                    {
                        Object oldFonts = oldResources->getVal(i);

                        if (oldFonts.isDict())
                        {
                            for (int f = 0; f < oldFonts.dictGetLength(); ++f)
                            {
                                fonts->add(oldFonts.dictGetKey(f), oldFonts.dictGetValNF(f).copy());
                            }
                        }
                    }
                    else
                    {
                        resources->add(key, oldResources->getValNF(i).copy());
                    }
                }
            }

            fonts->add("LBOcr", Object(fontRef));
            resources->add("Font", Object(fonts));

            pageObject.dictSet("Contents", Object(contents));
            pageObject.dictSet("Resources", Object(resources));
            xref->setModifiedObject(&pageObject, *pageRef);
        }

        QFile::remove(partial);

        if (document.saveAs(GooString(partial.toLocal8Bit().constData())) != errNone)
        {
            QFile::remove(partial);
            return false;
        }
    }

    return QFile::remove(pdfFile) && QFile::rename(partial, pdfFile);
}
