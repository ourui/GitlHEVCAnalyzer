#include "spsparser.h"
#include <QRegularExpression>

SpsParser::SpsParser(QObject *parent) :
    QObject(parent)
{
}

/** --- SAMPLE TEXT ---
  * Resolution:176x144
  * Max CU Size:64
  * Max CU Depth:4
  * Min TU Depth:0
  * Max TU Depth:1
  * Input Bit Depth:8
  */
bool SpsParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{

    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegularExpression cMatchTarget;


    // Resolution:176x144
    cMatchTarget.setPattern("Resolution:([0-9]+)x([0-9]+)");
    while( !pcInputStream->atEnd() ) {
        strOneLine = pcInputStream->readLine();
        auto match = cMatchTarget.match(strOneLine);
        if( match.hasMatch() ) {
            int iWidth  = match.captured(1).toInt();
            int iHeight = match.captured(2).toInt();
            pcSequence->setWidth(iWidth);
            pcSequence->setHeight(iHeight);
            break;
        }
    }



    // Max CU Size:64
    cMatchTarget.setPattern("Max CU Size:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        auto match = cMatchTarget.match(strOneLine);
        if( match.hasMatch() ) {
            int iMaxCUSize = match.captured(1).toInt();
            pcSequence->setMaxCUSize(iMaxCUSize);
            break;
        }
    }

    // Max CU Depth:4
    cMatchTarget.setPattern("Max CU Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        auto match = cMatchTarget.match(strOneLine);
        if( match.hasMatch() ) {
            int iMaxCUDepth = match.captured(1).toInt();
            pcSequence->setMaxCUDepth(iMaxCUDepth);
            break;
        }
    }

    // Max Inter TU Depth:3
    cMatchTarget.setPattern("Max Inter TU Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        auto match = cMatchTarget.match(strOneLine);
        if( match.hasMatch() ) {
            int iMinTUDepth = match.captured(1).toInt();
            pcSequence->setMaxInterTUDepth(iMinTUDepth);
            break;
        }
    }

    // Max Intra TU Depth:3
    cMatchTarget.setPattern("Max Intra TU Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        auto match = cMatchTarget.match(strOneLine);
        if( match.hasMatch() ) {
            int iMaxTUDepth = match.captured(1).toInt();
            pcSequence->setMaxIntraTUDepth(iMaxTUDepth);
            break;
        }
    }

    // Input Bit Depth:8
    cMatchTarget.setPattern("Input Bit Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        auto match = cMatchTarget.match(strOneLine);
        if( match.hasMatch()) {
            int iInputBitDepth = match.captured(1).toInt();
            pcSequence->setInputBitDepth(iInputBitDepth);
            break;
        }
    }

    return true;
}
