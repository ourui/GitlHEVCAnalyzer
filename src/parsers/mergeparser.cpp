#include "mergeparser.h"
#include <QRegularExpression>

MergeParser::MergeParser(QObject *parent) :
    QObject(parent)
{
}
bool MergeParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegularExpression cMatchTarget;


    /// <1,0> -1 0 0 -1 1
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<(-?[0-9]+),([0-9]+)> (.*)");
    QTextStream cMergeInfoStream;
    int iDecOrder = -1;
    int iLastPOC  = -1;
    while( !pcInputStream->atEnd() )
    {

        strOneLine = pcInputStream->readLine();
        auto match = cMatchTarget.match(strOneLine);
        if( match.hasMatch())
        {
            /// poc and lcu addr
            int iPoc = match.captured(1).toInt();
            iDecOrder += (iLastPOC != iPoc);
            iLastPOC = iPoc;

            pcFrame = pcSequence->getFramesInDecOrder().at(iDecOrder);
            int iAddr = match.captured(2).toInt();
            pcLCU = pcFrame->getLCUs().at(iAddr);

            ///
            QString strMergeInfo = match.captured(3);
            cMergeInfoStream.setString( &strMergeInfo, QIODeviceBase::ReadOnly );
            xReadMergeIndex(&cMergeInfoStream, pcLCU);

        }

    }
    return true;
}

bool MergeParser::xReadMergeIndex(QTextStream* pcMergeIndexStream, ComCU* pcCU)
{
    if( !pcCU->getSCUs().empty() )
    {
        /// non-leaf node : recursive reading for children
        xReadMergeIndex(pcMergeIndexStream, pcCU->getSCUs().at(0));
        xReadMergeIndex(pcMergeIndexStream, pcCU->getSCUs().at(1));
        xReadMergeIndex(pcMergeIndexStream, pcCU->getSCUs().at(2));
        xReadMergeIndex(pcMergeIndexStream, pcCU->getSCUs().at(3));
    }
    else
    {
        /// leaf node : read data
        int iMergeIndex;
        for(int i = 0; i < pcCU->getPUs().size(); i++)
        {
            Q_ASSERT(pcMergeIndexStream->atEnd() == false);
            *pcMergeIndexStream >> iMergeIndex;
            pcCU->getPUs().at(i)->setMergeIndex(iMergeIndex);
        }
    }
    return true;
}
