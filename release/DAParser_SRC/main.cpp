#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>

#include <iostream>
#include <string>

#define MTK_DA_MAGIC "MTK_DOWNLOAD_AGENT"

static quint32 get_l32(QByteArray input)
{
    QVector<quint32> res = {};
    for (uint i = 0; i < input.size()/sizeof(quint32); ++i)
        res.push_back(*(quint32*)(input.data()+i*sizeof(quint32)));

    return *res.data();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.isSetuidAllowed();

    qInfo("........ MTK Download Agent Parser .......");
    qInfo(".....................................................");

    if(argc < 2)
        qInfo("Drag and drop the da file to the exe");

    while (1) {

        QByteArray path(0xff, Qt::Uninitialized);
        std::cin.get((char*)path.data(), 0xff);

        qInfo(".....................................................");
        qInfo() << QString("Reading file %0").arg(path.data());

        QFile file(QDir::toNativeSeparators(path));
        if(!file.size())
        {
            qInfo().noquote() << QString("invalid file !(%0)").arg(qt_error_string());
            path.clear();
            return 0;
        }

        if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            qInfo().noquote() << QString("file open fail!(%0)").arg(qt_error_string());
            path.clear();
            return 0;
        }

        QByteArray magic = file.read(0x12);
        if(magic != MTK_DA_MAGIC)
        {
            qInfo().noquote() << QString("invalid MTK_DA file(hdr:%0)").arg((magic.data()));
            path.clear();
            return 0;
        }

        qInfo() << QString("Loading list supported chipset's by DA file");

        if (!file.seek(0x68))
            return 0;

        quint16 count_da = get_l32(file.read(4));
        for (quint16 i = 0; i <= count_da; i++)
        {
            if (!file.seek((0x6c + (0xdc * i))))
                return 0;

            QByteArray da_data = file.read(0xdc);
            QByteArray da_info = da_data.mid(0x2, 0x2).data();
            std::reverse(da_info.begin(), da_info.end());
            QString chip_id = da_info.toHex().simplified().data();
            if(!chip_id.size())
                continue;
            qInfo() << QString("MT%0").arg(chip_id);
        }

        path.clear();

        std::cin.ignore();
    }

    return a.exec();
}
