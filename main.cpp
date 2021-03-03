#include <QCoreApplication>
#include <QFile>
#include <QIODevice>
#include "datacontainer.h"
#include <QDebug>
#include "dtgen.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DataContainer d;

    QFile test("C:/Users/Pavel/Documents/build-DT-Desktop_Qt_5_15_0_MinGW_32_bit-Debug/debug/BSC03.LOG");
    test.open(QIODevice::ReadOnly);
    QString data = test.readAll();


    test.close();
    d.pushPrintout(data);
    //QFile test2("C:/Users/Pavel/Documents/build-DT-Desktop_Qt_5_15_0_MinGW_32_bit-Debug/debug/BSC03.LOG");
    //test2.open(QIODevice::ReadOnly);
    //QString data2 = test2.readAll();

    //test2.close();
    //d.pushPrintout(data2);

    //LUG911
    //LUG180
    //LUG245
    //LUG763
    //LU1861
    //LUG777
    //LUG778
    //auto rbsObject = d.getRbsObject("LUG778");

    DtGen gen = DtGen();

    for(QString rbs : d.getRbsList()){
        qDebug() << rbs;
        auto rbsObject = d.getRbsObject(rbs);
        QString tg = rbsObject["TG"].keys()[0];
        QString text;
        /*if(rbsObject["TG"][tg]["TMODE"][0] == "SCM"){
            text = gen.generate(rbsObject, "SIU");
            rbs.append(" SIU");
        } else {
            text = gen.generate(rbsObject);
        }*/
        text = gen.generate(rbsObject, "DEL");
        QFile out("DT Files/" + rbs + ".txt");
        out.open(QIODevice::WriteOnly);
        out.write(text.toLatin1());
        out.close();
    }
    qDebug() << "END";

    return a.exec();
}
