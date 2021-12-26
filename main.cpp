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
    DtGen gen = DtGen();

    for(QString rbs : d.getRbsList()){
        auto rbsObject = d.getRbsObject(rbs);
        QString tg = rbsObject["TG"].keys()[0];
        QString text;
        text = gen.generate(rbsObject, "DEL");
        QFile out("DT Files/" + rbs + ".txt");
        out.open(QIODevice::WriteOnly);
        out.write(text.toLatin1());
        out.close();
    }
    return a.exec();
}
