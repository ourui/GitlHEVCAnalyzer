#include "intraparser.h"
#include <QRegularExpression>
#include <QDebug>
IntraParser::IntraParser(QObject *parent) :
    QObject(parent)
{
}

bool IntraParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegularExpression cMatchTarget;


    /// <0,8> 8 36 31 36 30 36 31 36 0 36 2 36 1 36 0 36 1 36
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<(-?[0-9]+),([0-9]+)> (.*)");
    QTextStream cIntraDirInfoStream;
    int iDecOrder = -1;
    int iLastPOC  = -1;
    while( !pcInputStream->atEnd() )
    {

        strOneLine = pcInputStream->readLine();
        auto match = cMatchTarget.match(strOneLine);
        if( match.hasMatch() )
        {
            /// poc and lcu addr
            int iPoc = match.captured(1).toInt();
            iDecOrder += (iLastPOC != iPoc);
            iLastPOC = iPoc;

            pcFrame = pcSequence->getFramesInDecOrder().at(iDecOrder);
            int iAddr = match.captured(2).toInt();
            pcLCU = pcFrame->getLCUs().at(iAddr);


            ///
            QString strIntraDir = match.captured(3);
            cIntraDirInfoStream.setString( &strIntraDir, QIODeviceBase::ReadOnly );
            xReadIntraMode(&cIntraDirInfoStream, pcLCU);

        }

    }
    return true;
}


bool IntraParser::xReadIntraMode(QTextStream* pcCUInfoStream, ComCU* pcCU)
{
    if( !pcCU->getSCUs().empty() )
    {
        /// non-leaf node : recursive reading for children
        xReadIntraMode(pcCUInfoStream, pcCU->getSCUs().at(0));
        xReadIntraMode(pcCUInfoStream, pcCU->getSCUs().at(1));
        xReadIntraMode(pcCUInfoStream, pcCU->getSCUs().at(2));
        xReadIntraMode(pcCUInfoStream, pcCU->getSCUs().at(3));
    }
    else
    {
        /// leaf node : read data
        int iIntraDirLuma;
        int iIntraDirChroma;
        for(int i = 0; i < pcCU->getPUs().size(); i++)
        {
            Q_ASSERT(pcCUInfoStream->atEnd()==false);
            *pcCUInfoStream >> iIntraDirLuma >> iIntraDirChroma;
            pcCU->getPUs().at(i)->setIntraDirLuma(iIntraDirLuma);
            pcCU->getPUs().at(i)->setIntraDirChroma(iIntraDirChroma);
        }
    }
    return true;
}
