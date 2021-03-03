#ifndef DTGEN_H
#define DTGEN_H
#include "datacontainer.h"
#include <QFile>

struct GsmThread
{
    QString VcLayer;
    QString startDeviceSeries;
    QString endDeviceSeries;
    QString startDcpSeries;
    QString endDcpSeries;
    QString rbl;
    QString res64k;

    GsmThread()
    {
        VcLayer = "";
        startDeviceSeries = "";
        endDeviceSeries = "";
        startDcpSeries = "";
        endDcpSeries = "";
        rbl = "";
        res64k = "";
    }
};

class DtGen
{
public:
    DtGen();
    QString generate(QMap<QString, QMap<QString, QMap<QString, QStringList>>> rbsObject, QString templateName="TDM");

    void setTemplate(QString arg);

private:
    QString templatePath;
    QString templateText;
    QString containsTrxInfo(QString line);
    QString containsTransGroupInfo(QString line);
    QString containsChgr(QString line);
    QStringList sortedByLen(QStringList list);
    QString getTg(QString tgLine);
    QString getTrx(QString trxLine);
    QMap<QString, QStringList> getRblt2Series(QStringList rblt2);
    QString calcSdip(QString rbl);
    QString calcVcLayer(QString rbl);
    QList<GsmThread> getRblt264KSeries(QStringList &devices, QStringList &dcp, QStringList &s64K);
    QString addChgr(QString line, QMap<QString, QMap<QString, QMap<QString, QStringList>>> &rbsObj, QString cell, QString chgr);
    QString readTemplate(QString path);

    QStringList etmList;
    QMap<QString, QString> templates;
};

#endif // DTGEN_H
