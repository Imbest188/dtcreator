#ifndef DATACONTAINER_H
#define DATACONTAINER_H

#include <QMap>

struct GsmElement{
public:
    QString name;
    GsmElement(QString name){
        this->name = name;
    }
    QMap<QString, QStringList> params;
    void addParam(QString paramName, QString paramValue){
        params[paramName].append(paramValue);
    };
    QMap < QString, QStringList > getParams(){
        return params;
    }
};

struct Parametr{
public:
    QString name;
    int startPos;
    Parametr(QString name){
        this->name = name;
        startPos = 0;
    }
    Parametr(QString name, int startPos){
        this->name = name;
        this->startPos = startPos;
    }
    QStringList values;
};

class DataContainer
{
public:
    DataContainer();
    void pushPrintout(QString printData);
    QMap<QString,QStringList> getValues(QString itemName);
    QMap<QString,QStringList> getAllValues(QString itemName);
    QStringList getValues(QString itemName, QString paramName);
    QString findTransferingGroup(QString baseStationName);
    QMap<QString, QMap<QString, QMap<QString, QStringList>>> getRbsObject(QString objectName);
    QStringList getRbsList();

    QMap<QString,QMap<QString,QStringList>> elements;

private:
    QList<Parametr> getValues(QString & valueString, QList<Parametr> & params);
    void allocateValues(QList<Parametr> &);

    QList<Parametr> getParamsWithPos(QString &line);
    void checkIdentificator(QList<Parametr> &params);
    QString findParamValue(QString arg, QList<Parametr> &params);
    QString currentElement;
    QString currentChannelGroup;
    void printSlicer(QString &print);
    bool identificatorExists(QString identificatorName, QList<Parametr> &params);
};

#endif // DATACONTAINER_H
