#include "predparser.h"
#include <QRegularExpression>
#include <QDebug>

PredParser::PredParser(QObject *parent) :
    QObject(parent)
{
}

bool PredParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegularExpression cMatchTarget;


    /// <1,1> 0 0 1 1 0
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<(-?[0-9]+),([0-9]+)> (.*)");
    QTextStream cPredInfoStream;
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
            QString strCUInfo = match.captured(3);

            cPredInfoStream.setString(&strCUInfo, QIODeviceBase::ReadOnly );

            xReadPredMode(&cPredInfoStream, pcLCU);

        }

    }
    return true;
}


bool PredParser::xReadPredMode(QTextStream* pcPredInfoStream, ComCU* pcCU)
{
    if( !pcCU->getSCUs().empty() )
    {
        /// non-leaf node : recursive reading for children
        xReadPredMode(pcPredInfoStream, pcCU->getSCUs().at(0));
        xReadPredMode(pcPredInfoStream, pcCU->getSCUs().at(1));
        xReadPredMode(pcPredInfoStream, pcCU->getSCUs().at(2));
        xReadPredMode(pcPredInfoStream, pcCU->getSCUs().at(3));
    }
    else
    {
        /// leaf node : read data
        int iPredMode;
        for(int i = 0; i < pcCU->getPUs().size(); i++)
        {


            Q_ASSERT(pcPredInfoStream->atEnd() == false);

            *pcPredInfoStream >> iPredMode;
            ComPU* pcPU = pcCU->getPUs().at(i);
            pcPU->setPredMode((PredMode)iPredMode);
        }
    }
    return true;
}
