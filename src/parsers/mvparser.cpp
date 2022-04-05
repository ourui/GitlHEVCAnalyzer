#include "mvparser.h"
#include <QRegularExpression>

MVParser::MVParser(QObject *parent) :
    QObject(parent)
{
}

bool MVParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegularExpression cMatchTarget;


    /// <1,1> 1 -3 0 1 -3 0 1 -3 0 1 1 0 1 -3 0
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<(-?[0-9]+),([0-9]+)> (.*)");
    QTextStream cMVInfoStream;
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
            QString strMVInfo = match.captured(3);
            cMVInfoStream.setString( &strMVInfo, QIODeviceBase::ReadOnly );


            xReadMV(&cMVInfoStream, pcLCU);

        }

    }
    return true;
}
bool MVParser::xReadMV(QTextStream* pcMVInfoStream, ComCU* pcCU)
{
    if( !pcCU->getSCUs().empty() )
    {
        /// non-leaf node : recursive reading for children
        xReadMV(pcMVInfoStream, pcCU->getSCUs().at(0));
        xReadMV(pcMVInfoStream, pcCU->getSCUs().at(1));
        xReadMV(pcMVInfoStream, pcCU->getSCUs().at(2));
        xReadMV(pcMVInfoStream, pcCU->getSCUs().at(3));
    }
    else
    {
        /// leaf node : read data
        int iInterDir;
        for(int i = 0; i < pcCU->getPUs().size(); i++)
        {
            Q_ASSERT(!pcMVInfoStream->atEnd());
            *pcMVInfoStream >> iInterDir;
            ComPU* pcPU = pcCU->getPUs().at(i);
            pcPU->setInterDir(iInterDir);

            int iRefPOC;
            int iHor, iVer;
            ComMV* pcReadMV = NULL;
            if( iInterDir == 1 || iInterDir == 2)   //uni-prediction, 1 MV
            {
                *pcMVInfoStream >> iRefPOC >> iHor >> iVer;
                pcReadMV = new ComMV();
                pcReadMV->setRefPOC(iRefPOC);
                pcReadMV->setHor(iHor);
                pcReadMV->setVer(iVer);
                pcPU->getMVs().push_back(pcReadMV);
            }
            else if( iInterDir == 3 )               //bi-prediction, 2 MVs
            {
                *pcMVInfoStream >> iRefPOC >> iHor >> iVer;
                pcReadMV = new ComMV();
                pcReadMV->setRefPOC(iRefPOC);
                pcReadMV->setHor(iHor);
                pcReadMV->setVer(iVer);
                pcPU->getMVs().push_back(pcReadMV);

                *pcMVInfoStream >> iRefPOC >> iHor >> iVer;
                pcReadMV = new ComMV();
                pcReadMV->setRefPOC(iRefPOC);
                pcReadMV->setHor(iHor);
                pcReadMV->setVer(iVer);
                pcPU->getMVs().push_back(pcReadMV);
            }
        }
    }
    return true;
}
