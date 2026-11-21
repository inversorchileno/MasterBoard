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




#include "UBExportFullPDF.h"

#include <QtMath>

#include <QtCore>
#include <QtSvg>
#include <QPrinter>

#include "core/UBApplication.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBPersistenceManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPDFItem.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "pdf/GraphicsPDFItem.h"

#include "UBExportPDF.h"

#include <Merger.h>

#include "domain/UBGraphicsAttachmentItem.h"
#include "domain/UBGraphicsHyperlinkItem.h"
#include "domain/UBGraphicsCommentItem.h"
#include "domain/UBGraphicsSoundItem.h"
#include "pdf/UBPdfOcr.h"
#include <Exception.h>
#include <Transformation.h>

#include "core/memcheck.h"


using namespace merge_lib;


UBExportFullPDF::UBExportFullPDF(QObject *parent)
    : UBExportAdaptor(parent)
{
    // need to calculate screen resolution
    float dpiCommon = UBApplication::displayManager->logicalDpi(ScreenRole::Control);
    mScaleFactor = 72.0f / dpiCommon; // 1pt = 1/72 inch

    mSimpleExporter = new UBExportPDF();
}


UBExportFullPDF::~UBExportFullPDF()
{
    // NOOP
}


/** The clockwise turn of the page on the board: 0, 90, 180 or 270 degrees. */
static int quarterTurns(const UBGraphicsPDFItem* pdfItem)
{
    const QTransform placement = pdfItem->sceneTransform();
    return (qRound(qRadiansToDegrees(qAtan2(placement.m12(), placement.m11())) / 90.0) * 90 + 360) % 360;
}

/** The size of the page of the original PDF, as it is turned on the board. */
static QSizeF turnedPageSize(const UBGraphicsPDFItem* pdfItem)
{
    QSizeF size = pdfItem->pageSize();
    return quarterTurns(pdfItem) % 180 == 0 ? size : size.transposed();
}

QList<UBPdfAnnotations::Mark> UBExportFullPDF::pageMarks(std::shared_ptr<UBDocumentProxy> pDocumentProxy) const
{
    QList<UBPdfAnnotations::Mark> marks;
    const QList<int> pages = mSimpleExporter->pagesToExport(pDocumentProxy);

    for (int written = 0; written < pages.size(); ++written)
    {
        std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pages.at(written));

        if (!scene)
        {
            continue;
        }

        const QRectF page = scene->normalizedSceneRect();

        for (QGraphicsItem* item : scene->items())
        {
            UBPdfAnnotations::Mark mark;
            mark.page = written + 1;
            mark.place = QPointF((item->scenePos().x() - page.left()) / page.width(),
                                 (item->scenePos().y() - page.top()) / page.height());

            if (auto comment = qgraphicsitem_cast<UBGraphicsCommentItem*>(item))
            {
                mark.text = comment->text();
                mark.date = comment->date();
            }
            else if (auto attachment = qgraphicsitem_cast<UBGraphicsAttachmentItem*>(item))
            {
                mark.file = pDocumentProxy->persistencePath() + "/" + attachment->file();
            }
            else if (auto sound = qgraphicsitem_cast<UBGraphicsSoundItem*>(item))
            {
                mark.file = pDocumentProxy->persistencePath() + "/" + sound->file();
            }
            else if (auto hyperlink = qgraphicsitem_cast<UBGraphicsHyperlinkItem*>(item))
            {
                // Other readers show a comment with the address.
                mark.text = hyperlink->address();
            }
            else
            {
                continue;
            }

            marks << mark;
        }
    }

    return marks;
}

void UBExportFullPDF::setPages(const QList<int>& pages)
{
    mSimpleExporter->setPages(pages);
}

void UBExportFullPDF::saveOverlayPdf(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const QString& filename)
{
    if (!pDocumentProxy || filename.length() == 0 || pDocumentProxy->pageCount() == 0)
        return;

    /*
        PDFMerger is supposed to be working only for PDFs using 1.0 to 1.4 standard, but I encountered no issue using 1.7 documents.
        Just tested with a few documents though so it could be a total mess to restore it without any possibility to disable it if an issue is encountered
        A new setting is introduced to handle that possibility.
        Also, calling simpleExporter here instead of the following code was done because I (wrongly) assumed that both codes was doing nearly the same thing, as I never
        tested to remove the part of pdf-merger throwing an exception if pdf version is > 1.4 and that calling the following lines without modifying the version check
        throws an exception and finally calls simpleExporter->persistsDocument
        calling simpleExporter directly also resulted in fixing two issues where pdf overlay could be badly scaled (not matching with scale of annotations)
    */
    bool usePDFMerger = UBSettings::settings()->pdfUsePDFMerger->get().toBool();
    if (usePDFMerger)
    {
        //PDF
        qDebug() << "exporting document to PDF Merger" << filename;
        QPrinter pdfPrinter;

        pdfPrinter.setOutputFormat(QPrinter::PdfFormat);
        pdfPrinter.setResolution(UBSettings::settings()->pdfResolution->get().toInt());
        pdfPrinter.setOutputFileName(filename);
        pdfPrinter.setFullPage(true);

        QPainter* pdfPainter = 0;

        const QList<int> pages = mSimpleExporter->pagesToExport(pDocumentProxy);

        for(int written = 0 ; written < pages.size(); written++)
        {
            const int pageIndex = pages.at(written);
            std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
            // set background according to PDF export settings
            bool isDark = scene->isDarkBackground();
            UBPageBackground pageBackground = scene->pageBackground();

            bool exportDark = isDark && UBSettings::settings()->exportBackgroundColor->get().toBool();

            bool sceneHasPDFBackground = false;

            // set high res rendering
            scene->setRenderingQuality(UBItem::RenderingQualityHigh, UBItem::CacheNotAllowed);
            scene->setRenderingContext(UBGraphicsScene::PdfExport);

            // pageSize is the output PDF page size; it is set to equal the scene's boundary size; if the contents
            // of the scene overflow from the boundaries, they will be scaled down.
            QSizeF pageSize = scene->sceneSizeF() * mScaleFactor;   // points

            UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());

            if (pdfItem)
            {
                mHasPDFBackgrounds = true;
                sceneHasPDFBackground = true;
                pageSize = turnedPageSize(pdfItem);     // original PDF document page size, as turned on the board
            }
            else
            {
                sceneHasPDFBackground = false;
            }

            QPageSize size(pageSize, QPageSize::Point);
            pdfPrinter.setPageSize(size);

            if (!pdfPainter) pdfPainter = new QPainter(&pdfPrinter);

            if (written != 0) pdfPrinter.newPage();

            // do not draw background color and grid if scene has PDF background
            if (sceneHasPDFBackground)
            {
                scene->setDrawingMode(true);
                scene->setBackground(false, UBPageBackground::plain);
            }
            else if (UBSettings::settings()->exportBackgroundGrid->get().toBool())
            {
                scene->setBackground(exportDark, pageBackground);
            }
            else
            {
                scene->setBackground(exportDark, UBPageBackground::plain);
            }

            //render to PDF
            scene->render(pdfPainter, QRectF(), scene->normalizedSceneRect());

            //restore screen rendering quality
            scene->setRenderingContext(UBGraphicsScene::Screen);
            scene->setRenderingQuality(UBItem::RenderingQualityNormal, UBItem::CacheAllowed);

            //restore background state
            scene->setDrawingMode(false);
            scene->setBackground(isDark, pageBackground);
        }

        if (pdfPainter) delete pdfPainter;
    }
    else
    {
        mSimpleExporter->persistsDocument(pDocumentProxy, filename);
    }
}


void UBExportFullPDF::persist(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    persistLocally(pDocumentProxy, tr("Export as PDF File"));
}


bool UBExportFullPDF::persistsDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const QString& filename)
{
    QFile file(filename);
    if (file.exists()) file.remove();

    QString overlayName = filename;
    overlayName.replace(".pdf", "_overlay.pdf");

    QFile previousOverlay(overlayName);
    if (previousOverlay.exists())
        previousOverlay.remove();

    mHasPDFBackgrounds = false;

    saveOverlayPdf(pDocumentProxy, overlayName);

    if (!mHasPDFBackgrounds)
    {
        QFile f(overlayName);
        f.rename(filename);
    }
    else
    {
        Merger merger;
        try
        {
            merger.addOverlayDocument(QFile::encodeName(overlayName).constData());

            MergeDescription mergeInfo;
            QHash<QString, UBPdfOcr::Pages> ocrOfFile;
            QList<UBPdfAnnotations::TextPage> textPages;

            // factor between scene coordinates and PDF coordinates
            double dpiScale = 72. / pDocumentProxy->pageDpi();

            const QList<int> pages = mSimpleExporter->pagesToExport(pDocumentProxy);

            for(int written = 0 ; written < pages.size(); written++)
            {
                const int pageIndex = pages.at(written);
                std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
                UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());

                if (pdfItem)
                {
                    QString pdfName = UBPersistenceManager::objectDirectory + "/" + pdfItem->fileUuid().toString() + ".pdf";
                    QString backgroundPath = pDocumentProxy->persistencePath() + "/" + pdfName;

                    // Original data in scene coordinates, annotationsRect always contains pdfSceneRect
                    QRectF pdfSceneRect = pdfItem->sceneBoundingRect();
                    QRectF annotationsRect = scene->normalizedSceneRect();

                    double xAnnotation = annotationsRect.x();
                    double yAnnotation = annotationsRect.y();
                    double xPdf = pdfSceneRect.x();
                    double yPdf = pdfSceneRect.y();
                    double hPdf = pdfSceneRect.height();

                    // Exportation-transformed data, scaleFactor always <= 1
                    double hScaleFactor = pdfSceneRect.width() / annotationsRect.width();
                    double vScaleFactor = pdfSceneRect.height() / annotationsRect.height();
                    double scaleFactor = qMin(hScaleFactor, vScaleFactor);

                    double xAnnotationsOffset = 0;
                    double yAnnotationsOffset = 0;
                    double hPdfTransformed = hPdf * scaleFactor;

                    // Compute scaling of PDF on the scene
                    // If the PDF was scaled when added to the scene (e.g if it was loaded from a document with a different DPI
                    // than the current one), it should also be scaled here.
                    // A page turned on the board is turned in the PDF too:
                    // the merger turns it counterclockwise, the board clockwise.
                    const int turn = quarterTurns(pdfItem);
                    const QSizeF pageSize = turnedPageSize(pdfItem);

                    double pdfScale = pdfSceneRect.width() / pageSize.width() * dpiScale;

                    // Offsets are calculated in the PDF coordinate system.
                    // It has its origin at the lower left corner and is measured in points of 1/72 inch.
                    // Here, we force the PDF page to be on the topleft corner of the page
                    double xPdfOffset = 0;
                    double yPdfOffset = (hPdf - hPdfTransformed) * dpiScale / pdfScale;

                    // Now we align the items
                    xPdfOffset += (xPdf - xAnnotation) * scaleFactor * dpiScale / pdfScale;
                    yPdfOffset -= (yPdf - yAnnotation) * scaleFactor * dpiScale / pdfScale;

                    TransformationDescription pdfTransform(xPdfOffset, yPdfOffset, scaleFactor, (360 - turn) % 360);
                    TransformationDescription annotationTransform(xAnnotationsOffset, yAnnotationsOffset, 1, 0);

                    MergePageDescription pageDescription(pageSize.width(),
                                                         pageSize.height(),
                                                         pdfItem->pageNumber(),
                                                         QFile::encodeName(backgroundPath).constData(),
                                                         pdfTransform,
                                                         written + 1,
                                                         annotationTransform,
                                                         false, false);

                    mergeInfo.push_back(pageDescription);

                    merger.addBaseDocument(QFile::encodeName(backgroundPath).constData());

                    // The words read on a scanned page go under its picture, as text.
                    if (!ocrOfFile.contains(backgroundPath))
                    {
                        ocrOfFile.insert(backgroundPath, UBPdfOcr::load(backgroundPath));
                    }

                    const QList<UBOcrEngine::Word> words = ocrOfFile.value(backgroundPath).value(pdfItem->pageNumber());

                    if (!words.isEmpty())
                    {
                        UBPdfAnnotations::TextPage textPage;
                        textPage.page = written + 1;
                        textPage.words = words;
                        textPage.sourceSize = pdfItem->pageSize();
                        textPage.turn = turn;
                        // The page of the PDF fills the page written, smaller when
                        // annotations stand out of it, at its top left.
                        textPage.placement = QRectF((xPdf - xAnnotation) * scaleFactor / pdfSceneRect.width(),
                                                    (yPdf - yAnnotation) * scaleFactor / pdfSceneRect.height(),
                                                    scaleFactor, scaleFactor);
                        textPages << textPage;
                    }
                }
                else
                {
                    QSizeF pageSize = scene->nominalSize() * mScaleFactor;

                    MergePageDescription pageDescription(pageSize.width(),
                             pageSize.height(),
                             0,
                             "",
                             TransformationDescription(),
                             written + 1,
                             TransformationDescription(),
                             false, true);

                    mergeInfo.push_back(pageDescription);
                }
            }

            merger.merge(QFile::encodeName(overlayName).constData(), mergeInfo);

            merger.saveMergedDocumentsAs(QFile::encodeName(filename).constData());

            if (!UBPdfAnnotations::writeTextLayer(filename, textPages))
            {
                qWarning() << "The recognized text could not be written into" << filename;
            }

        }
        catch(const std::exception& e)
        {
            qWarning() << "An exception occured during PDF merging of document " << filename << " :" << e.what();

            QMessageBox errorBox;
            errorBox.setIcon(QMessageBox::Warning);
            errorBox.setText(tr("The original PDF imported in Master Board seems not valid and could not be merged with your annotations. Please repair it and then reimport it in Master Board. The current export will be done with detailed (heavy) images of the pages of the original PDF instead, to avoid complete export failure."));
            QApplication::restoreOverrideCursor();
            errorBox.exec();
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            // default to raster export

            mSimpleExporter->persistsDocument(pDocumentProxy, filename);
        }
        catch (...)
        {
            qWarning() << "An exception occured during PDF merging of document " << filename;

            QMessageBox errorBox;
            errorBox.setIcon(QMessageBox::Warning);
            errorBox.setText(tr("The original PDF imported in Master Board seems not valid and could not be merged with your annotations. Please repair it and then reimport it in Master Board. The current export will be done with detailed (heavy) images of the pages of the original PDF instead, to avoid complete export failure."));
            QApplication::restoreOverrideCursor();
            errorBox.exec();
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            // default to raster export
            mSimpleExporter->persistsDocument(pDocumentProxy, filename);
        }

        if (!UBApplication::app()->isVerbose())
        {
            QFile::remove(overlayName);
        }
    }


    // The quick notes and the files of the pages, as annotations other PDF readers show.
    if (!UBPdfAnnotations::write(filename, pageMarks(pDocumentProxy)))
    {
        qWarning() << "The quick notes and the files could not be written into" << filename;
    }
    return true;
}

bool UBExportFullPDF::associatedActionactionAvailableFor(const QModelIndex &selectedIndex)
{
    const UBDocumentTreeModel *docModel = qobject_cast<const UBDocumentTreeModel*>(selectedIndex.model());
    if (!selectedIndex.isValid() || docModel->isCatalog(selectedIndex)) {
        return false;
    }

    return true;
}


QString UBExportFullPDF::exportExtention()
{
    return QString(".pdf");
}

QString UBExportFullPDF::exportName()
{
    return tr("Export to PDF");
}
