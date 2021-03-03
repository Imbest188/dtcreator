#include "datacontainer.h"

DataContainer::DataContainer()
{

}

void DataContainer::pushPrintout(QString printData)
{
    printSlicer(printData);
}

QList<Parametr> DataContainer::getValues(QString &valueString, QList<Parametr> &params)
{
    int lastElement = params.size();
    int start = 0;
    int end = 0;
    QList<Parametr> result;
    for(int i = 0; i < lastElement; i++){
        start = params[i].startPos;
        if(i < lastElement - 1){
            end = params[i+1].startPos;
        } else {
            end = valueString.size() - 1;
        }
        QString value = valueString.mid(start, end - start).trimmed();
        // Проверка на пустые значения
        Parametr newParam = Parametr(params[i].name);
        newParam.values.append(value);
        result.append(newParam);
    }
    return result;
}
#include <QDebug>
void DataContainer::allocateValues(QList<Parametr> &params)
{
    checkIdentificator(params);
    for(auto param : params){
        if(param.name == "CHGR"){
            if(currentChannelGroup.size()){
                for(auto subParam : params){
                    subParam.values.removeDuplicates();
                    subParam.values.removeAll("");
                    elements[currentElement + currentChannelGroup][subParam.name] += subParam.values;
                }
                elements[currentElement + currentChannelGroup]["CELL"] += currentElement;
            }
        }
        if(QStringList({"CELL","MO"}).contains(param.name)){
            if(elements[currentElement][param.name].isEmpty())
                elements[currentElement][param.name] += param.values;
        } else {
            //param.values.removeAll("");
            elements[currentElement][param.name] += param.values;
        }
    }
}

QList<Parametr> DataContainer::getParamsWithPos(QString &line)
{
    QList<Parametr> result;
    QStringList parts = line.split(' ');
    for(QString part : parts){
        QString name = part.trimmed();
        if(name.size()){
            int from = 0;
            if(result.size()){
                from = result.last().startPos + result.last().name.size();
            }
            int pos = line.indexOf(name, from);
            result.append(Parametr(name, pos));
        }
    }
    return result;
}

void DataContainer::checkIdentificator(QList<Parametr> &params)
{
    for(auto param : params){ // CHGR check
        if(param.name == "CHGR"){
            QString newChgr = findParamValue("CHGR", params);
            if(newChgr.size())
                currentChannelGroup = newChgr;
        }
    }
    QStringList elementIdentificator({"MO", "CELL", "PSTU", "SCGR"});
    for(auto identificator : elementIdentificator){
        QString newIdentificator = findParamValue(identificator, params);
        if(newIdentificator.size()){
            currentElement = newIdentificator;
            return;
        } else {
            if(identificatorExists(identificator, params))
                return; //Пустой MO считается предыдущим
        }
    }
}

QString DataContainer::findParamValue(QString paramName, QList<Parametr> &params)
{
//    if(paramName == "SCGR"){
//        for(auto par : params)
//            //if(par.name == "SCGR")
//                qDebug() << par.name << par.values << par.startPos;
//    }
    for(auto param : params){
        if(param.name == paramName){
            if(param.values.size()){
                return param.values[0];
            }
        }
    }
    return "";
}

void DataContainer::printSlicer(QString &print)
{
    QStringList lines = print.split('\n');
    QList<Parametr> params;
    bool valueLines = false;
    QStringList specialValues({"SCTYPE", "CELL"});
    for(QString line : lines){
        if(line.trimmed().size()){
            bool specialValue = false;
            for(QString val : specialValues){
                if(line.contains(val)){
                    specialValue = true;
                    break;
                }
            }
            if(valueLines && specialValue == false){
                auto values = getValues(line, params);
                allocateValues(values);
            } else {
                params = getParamsWithPos(line);
                valueLines = true;
            }
        } else {
            valueLines = false;
        }
    }
}

bool DataContainer::identificatorExists(QString identificatorName, QList<Parametr> &params)
{
    for(auto param : params){
        if(param.name == identificatorName){
            return true;
        }
    }
    return false;
}

QMap<QString, QStringList> DataContainer::getValues(QString itemName)
{
    if(elements[itemName].size()){
        auto result = elements[itemName];
        for(auto it=result.begin(); it!=result.end(); it++){
            it.value().removeAll("");
        }
        return result;
    }
    return QMap<QString, QStringList>();
}

QMap<QString, QStringList> DataContainer::getAllValues(QString itemName)
{
    if(elements[itemName].size())
        return elements[itemName];
    return QMap<QString, QStringList>();
}

QStringList DataContainer::getValues(QString itemName,QString paramName)
{
    if(elements[itemName][paramName].size())
        return elements[itemName][paramName];
    return QStringList();
}

QString DataContainer::findTransferingGroup(QString baseStationName)
{
    for(auto it=elements.begin(); it!=elements.end(); it++){
      if(it->contains("RSITE") && it.value()["RSITE"][0] == baseStationName){
          return it.key();
      }
    }
    return "";
}

QMap<QString, QMap<QString, QMap<QString, QStringList>>> DataContainer::getRbsObject(QString objectName)
{
    QMap<QString, QMap<QString, QMap<QString, QStringList>>> result;
    QString tg = findTransferingGroup(objectName);

    if(tg.size() == 0)
        return QMap<QString, QMap<QString, QMap<QString, QStringList>>>();
    auto tgObj = getAllValues(tg);
    tgObj.remove("APUSAGE");
    tgObj.remove("APSTATE");

    auto pstuObj = getValues(objectName);
    pstuObj["PSTU"].removeDuplicates();
    result["PSTU"].insert("PSTU", pstuObj);
    if(pstuObj.keys().contains("SCGR")){
        QString scgr = pstuObj["SCGR"][0];
        auto scgrObj = getValues(scgr);
        result["SCGR"].insert(scgr, scgrObj);
    }

    result["TG"].insert(tg, tgObj);
    QString intTg = tg.replace("RXOTG-","");
    for(QString trxSubobject : QStringList({"RXOTRX","RXOTX","RXORX"})){
        for(int i = 0; i < 12; i++){
            QString name = trxSubobject + '-' + intTg + '-' + QString::number(i);
            auto trxObj = getValues(name);
            if(trxObj.size()){
                result[trxSubobject].insert(name, trxObj);
            }
        }
    }

    for(QString tgSubobject : QStringList({"RXOCON","RXOCF","RXOIS","RXOTF"})){
        QString name = tgSubobject + '-' + intTg;
        auto tgSubObj = getValues(name);
        if(tgSubObj.size()){
            result[tgSubobject].insert(name, tgSubObj);
        }
    }

    for(char cellIdent : {'A','B','C'}){
        QString cellName = objectName + cellIdent;
        auto cellObj = getValues(cellName);
        cellObj["CHGR"].removeDuplicates();
        if(cellObj.size()){
            result["CELL"].insert(cellName, cellObj);
        }
        for(auto chgr : cellObj["CHGR"]){
            QString groupName = cellName+chgr;
            auto values = getValues(groupName);
            for(auto val : values){
                val.removeDuplicates();
                val.removeAll("");
            }
            if(chgr.size())
                result[cellName].insert(chgr, values);
        }

    }
    return result;
}

QStringList DataContainer::getRbsList()
{
    QStringList result;
    for(auto it=elements.begin(); it!=elements.end(); it++){
      if(it->contains("RSITE")){
          result.append(it.value()["RSITE"][0]);
      }
    }
    result.sort();
    return result;
}
