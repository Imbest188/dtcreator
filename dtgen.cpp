#include "dtgen.h"

DtGen::DtGen()
{
    templates.insert("TDM", readTemplate("C:/Users/Pavel/Documents/build-DT-Desktop_Qt_5_15_0_MinGW_32_bit-Debug/debug/template.txt"));
    templates.insert("SIU", readTemplate("C:/Users/Pavel/Documents/build-DT-Desktop_Qt_5_15_0_MinGW_32_bit-Debug/debug/template siu.txt"));
    templates.insert("DEL", readTemplate("C:/Users/Pavel/Documents/build-DT-Desktop_Qt_5_15_0_MinGW_32_bit-Debug/debug/delete template.txt"));

    etmList = QStringList({"1", "3","4","5","7","8","9","10","11","13","14","15","16","17",});
    setTemplate("TDM");
}
#include <QDebug>
QString DtGen::generate(QMap<QString, QMap<QString, QMap<QString, QStringList> > > rbsObject, QString templateName)
{
    setTemplate(templateName);
    QStringList result;
    QString allCells = rbsObject["CELL"].keys().join('&');
    for(QString line : templateText.split('\n')){

        if(line.contains("$CELLS$")){
            line.replace("$CELLS$", allCells);
        }

        if(line.contains("$RXOTG$") || line.contains("$TG$")){
            QString tgName = rbsObject["TG"].keys()[0];
            if(line.contains("$RSITE$")){
                line.replace("$RSITE$",rbsObject["TG"][tgName]["RSITE"][0]);
            }
            for(QString word : line.split(' ')){
                if(word == "$RXOTG$"){
                    line.replace(word, tgName);
                } else if (word == "$TG$"){
                    line.replace(word, getTg(tgName));
                } else {
                    if(word.contains('$') && QStringList({"$CELL$", "$CHGR$"}).contains(word) == false){
                        QStringList values = rbsObject["TG"][tgName][word.mid(1, word.size() - 2)];
                        values.removeDuplicates();
                        QString value = values.join('&');
                        if(value.size())
                            line.replace(word, value);
                    }
                }
            }
        }

        if(line.contains("$PSTU$") || line.contains("$SCGR$") || line.contains("ABISALLOC")){
            auto pstuObj = rbsObject["PSTU"]["PSTU"];
            auto scgrObj = rbsObject["SCGR"][pstuObj["SCGR"][0]];
            if(line.contains("$SC$")){
                for(int sc = 0; sc < scgrObj["SC"].size(); sc++){
                    QString newLine = line;

                    newLine.replace("$SCGR$",pstuObj["SCGR"][0]);
                    newLine.replace("$SC$", scgrObj["SC"][sc]);
                    newLine.replace("$NUMDEV$", scgrObj["NUMDEV"][sc]);
                    newLine.replace("$DCP$", scgrObj["DCP"][sc]);

                    result.append(newLine);
                }
                line = "None";
            }
            for(QString word : line.split(' ')){
                if(word.contains('$')){
                    QString newWord = pstuObj[word.mid(1, word.size() - 2)].join('&');
                    if(newWord.size()){

                    } else {
                        newWord = scgrObj[word.mid(1, word.size() - 2)].join('&');
                    }
                    line.replace(word, newWord);
                }
            }
        }

        QStringList dipArgs({"$RBL2$","$ADEVS$","$ETM$","$DEVS$"});
        for(QString tok : dipArgs){
            if(line.contains(tok)){
                QString tgName = rbsObject["TG"].keys()[0];
                auto threadObj = rbsObject["TG"][tgName];
                QStringList resLine;
                auto devSeries = getRblt2Series(threadObj["DEV"]);
                if(line.contains("$ADEVS$")){
                    for(auto series : devSeries.keys()){
                        QString newLine = line;
                        QString seriesValue = "-" + devSeries[series].join("&&-");
                        newLine.replace("$ADEVS$", seriesValue);

                        resLine.append(newLine);
                    }
                } else {
                    for(GsmThread thread : getRblt264KSeries(threadObj["DEV"],threadObj["DCP"],threadObj["64K"])){
                        QString newLine = line;
                        newLine.replace("$DEVS$", QString("RBLT2-" + thread.startDeviceSeries + "&&-" + thread.endDeviceSeries));
                        newLine.replace("$DCPS$", QString(thread.startDcpSeries + "&&" + thread.endDcpSeries));
                        newLine.replace("$ETM$", calcSdip(thread.rbl));
                        newLine.replace("$LP$", calcVcLayer(thread.rbl));
                        newLine.replace("$RBL2$", thread.rbl);
                        if(thread.res64k == "YES" && newLine.contains("RXAPI")){
                            newLine.replace(';', ", RES64K ;");
                        }

                        resLine.append(newLine);
                    }
                }
                line = "None";
                //resLine.replace("\n\n", "\n");
                result.append(resLine);
                break;
            }
        }

        QString tgObj = containsTransGroupInfo(line);
        if(tgObj.size()){
            QString name = getTg(rbsObject["TG"].keys()[0]);
            for(QString word : line.split(' ')){
                if(QStringList({"$RXOCF$","$RXOCON$","$RXOIS$","$RXOTF$"}).contains(word)){
                    line.replace(word, name);
                } else if(word.contains('$')){
                    QString value = rbsObject[tgObj][name][word.mid(1, word.size() - 2)].join('&');
                    if(value.size()){
                        line.replace(word, value);
                    }
                }
            }
        }

        QString trxObj = containsTrxInfo(line);
        if(trxObj.size()){
            if(trxObj == "TRX"){
                trxObj = "RXOTRX";
            }
            for(auto rxotrx : sortedByLen(rbsObject[trxObj].keys())){
                QString newLine = line;
                newLine.replace('$' + trxObj + '$', rxotrx);
                for(QString word : line.split(' ')){
                    if(word.contains('$')){
                        QString value;
                        if(word == "$TRX$"){
                            value = getTrx(rxotrx);
                        } else {
                            value = rbsObject[trxObj][rxotrx][word.mid(1, word.size() - 2)].join('&');
                        }
                        if(value.size()){
                            newLine.replace(word, value);
                        }

                    }
                }
                result.append(newLine);
            }
        } else if(line.contains("$CELL$")){
            for(QString cell : rbsObject["CELL"].keys()){
                for(QString group : rbsObject["CELL"][cell]["CHGR"]){
                    QString newLine = addChgr(line, rbsObject, cell, group);
                    if(containsChgr(line).size() == 0 && group != "0"){

                    } else {
                        for(QString word : newLine.split(' ')){
                            if(word.contains('$')){
                                QString value;
                                QStringList listValues = rbsObject["CELL"][cell][word.mid(1, word.size() - 2)];
                                listValues.removeDuplicates();
                                value = listValues.join('&');
                                if(word == "$NCCPERM$"){
                                    value = value.split("  ").join('&');
                                }
                                if(value.size()){
                                    newLine.replace(word, value);
                                }
                            }
                        }
                        result.append(newLine);
                    }
                }
            }
        }
        else
            result.append(line);
    }

    result.removeAll("None");
    return result.join('\n').toLatin1().replace(", ",",").replace(" -","-").replace("- ","-").replace(" =","=");
}

void DtGen::setTemplate(QString arg)
{
    if(templates.keys().contains(arg)){
        templateText = templates[arg];
    }
}

QString DtGen::containsTrxInfo(QString line)
{
    for(QString tok : QStringList({"$RXOTRX","$RXORX","$RXOTX","$TRX"})){
        if(line.contains(tok))
            return tok.mid(1);
    }
    return "";
}

QString DtGen::containsTransGroupInfo(QString line)
{
    for(QString tok : QStringList({"$RXOCF","$RXOCON","$RXOIS","$RXOTF"})){
        if(line.contains(tok))
            return tok.mid(1);
    }
    return "";
}

QString DtGen::containsChgr(QString line)
{
    for(QString tok : QStringList({"$CHGR>0","$CHGR"})){
        if(line.contains(tok))
            return tok.mid(1);
    }
    return "";
}

QStringList DtGen::sortedByLen(QStringList list)
{
    if(list.size() < 7)
        return list;
    int defLen = list[0].size();
    QStringList newList;
    for(int i = 0; i < list.size(); i++){
        if(list[i].size() == defLen){
            newList.append(list[i]);
        }
    }
    for(int i = 0; i < list.size(); i++){
        if(list[i].size() > defLen){
            newList.append(list[i]);
        }
    }
    return newList;
}

QString DtGen::getTg(QString tgLine)
{
    if(tgLine.size() > 6){
        return tgLine.mid(6);
    }
    return tgLine;
}

QString DtGen::getTrx(QString trxLine)
{
    if(trxLine.size() > 7){
        return trxLine.mid(7);
    }
    return trxLine;
}

QString DtGen::calcSdip(QString rbl)
{
    int rblNum = rbl.toInt();
    if(rblNum >= 0){
        int pos = rblNum / 64;
        if(pos < etmList.size())
            return etmList[pos] + " ETM2";
    }
    return "";
}

QString DtGen::calcVcLayer(QString rbl)
{
    int rblNum = rbl.toInt();
    if(rblNum >= 0){
        return "VC12-" + QString::number(rblNum % 64);
    }
    return "";
}

QList<GsmThread> DtGen::getRblt264KSeries(QStringList &devices, QStringList &dcp, QStringList &s64K) //rewrite
{
    QList<GsmThread> result;
    if(devices.size() == dcp.size() && dcp.size() == s64K.size()){

    } else {
        qDebug() << "Wrong size RXAPP(DEV)";
        return result;
    }
    int last = -1;
    QString lastDevices;
    QString lastDcp;
    QString last64K;
    GsmThread thread;
    for(int it = 0; it < devices.size(); it++){
        if(devices[it].size()){
            QString dev = devices[it];
            QString dcpNum = dcp[it];
            QString dev64k = s64K[it];
            QStringList numPart = dev.split('-');
            if(numPart.size() > 1){
                int rbl = numPart[1].toInt() / 32;
                if(last != rbl || dev64k != last64K){
                    if(last > -1){
                        thread.endDeviceSeries = lastDevices;
                        thread.endDcpSeries = lastDcp;
                        result.append(thread);
                        thread = GsmThread();
                    }
                    thread.rbl = QString::number(rbl);
                    thread.startDeviceSeries = numPart[1];
                    thread.res64k = dev64k;
                    thread.startDcpSeries = dcpNum;

                }
                last = rbl;
                lastDevices = numPart[1];
                lastDcp = dcpNum;
                last64K = dev64k;
            }
        }
    }
    thread.endDeviceSeries = lastDevices;
    thread.endDcpSeries = lastDcp;
    result.append(thread);
    return result;
}

QMap<QString, QStringList> DtGen::getRblt2Series(QStringList rblt2)
{
    QMap<QString, QStringList> result;
    int last = -1;
    QString lastDevices;
    for(QString dev : rblt2){
        QStringList numPart = dev.split('-');
        if(numPart.size() > 1){
            int rbl = numPart[1].toInt() / 32;
            if(last != rbl){
                if(result.keys().contains(QString::number(rbl))){

                } else {
                    result[QString::number(rbl)].append(numPart[1]);
                }
                if(last > -1){
                    result[QString::number(last)].append(lastDevices);
                }
                last = rbl;
            } else {

            }
            lastDevices = numPart[1];
        }
    }
    result[QString::number(last)].append(lastDevices);
    return result;
}

QString DtGen::addChgr(QString line, QMap<QString, QMap<QString, QMap<QString, QStringList> > > &rbsObj, QString cell, QString chgr)
{
    QString chgrToken = containsChgr(line);
    if(chgrToken.size()){
        if(chgrToken.contains(">0") && chgr == "0"){
            return "None";
        }

        line.replace(">0$", "$");
        for(QString word : line.split(' ')){
            if(word.contains('$')){
                QString key = word.mid(1, word.size() - 2);
                if(rbsObj[cell][chgr].contains(key)){
                    QStringList val = rbsObj[cell][chgr][key];
                    if(key == "DCHNO"){
                        val.removeAll(rbsObj["CELL"][cell]["BCCHNO"][0]); // del bcch from other frequencies
                    }
                    val.removeDuplicates();
                    QString value = val.join(('&'));
                    if(value.size()){
                        line.replace(word, value);
                    }
                }
            }
        }
    }
    return line;
}

QString DtGen::readTemplate(QString path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QString result = file.readAll();
    file.close();
    return result;
}
