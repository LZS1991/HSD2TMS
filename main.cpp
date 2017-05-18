#include <QCoreApplication>
#include "hsd2tms.h"
#include <iostream>
#include <chrono>

//如果使用VS编译器，需要将代码强制转换为UTF-8,
//并将文件保存为UTF-8编码，才能正确显示中文
#if defined(_MSC_VER)&&(_MSC_VER>=1600)
#pragma execution_character_set("utf-8")
#endif

namespace hsd2tms {
void createTiles(uint32_t aMaxZoomLevel, const HimawariStandardData& aData,
                 DataType aType,
                 uint32_t aBand0, uint32_t aBand1, uint32_t aBand2) {
    if (!aData.mBands[aBand0].hasData() ||
            !aData.mBands[aBand1].hasData() ||
            !aData.mBands[aBand2].hasData()) {
        return;
    }

    int32_t z = aMaxZoomLevel;
    uint32_t max = 1 << z;
    for (uint32_t x = 0; x < max; x++) {
        for (uint32_t y = 0; y < max; y++) {
            createTile(z, x, y, aData, aType, aBand0, aBand1, aBand2);
        }
    }
}

void createTiles(uint32_t aMaxZoomLevel, const HimawariStandardData& aData,
                 DataType aType,
                 uint32_t aBand0, uint32_t aBand1) {
    if (!aData.mBands[aBand0].hasData() ||
            !aData.mBands[aBand1].hasData()) {
        return;
    }

    int32_t z = aMaxZoomLevel;
    uint32_t max = 1 << z;
    for (uint32_t x = 0; x < max; x++) {
        for (uint32_t y = 0; y < max; y++) {
            createTile(z, x, y, aData, aType, aBand0, aBand1);
        }
    }
}

void createTiles(uint32_t aMaxZoomLevel, const HimawariStandardData& aData,
                 DataType aType,
                 uint32_t aBand) {
    if (!aData.mBands[aBand].hasData()) {
        return;
    }

    int32_t z = aMaxZoomLevel;
    uint32_t max = 1 << z;
    for (uint32_t x = 0; x < max; x++) {
        for (uint32_t y = 0; y < max; y++) {
            createTile(z, x, y, aData, aType, aBand);
        }
    }
}

// lv8 Etorofu: 8/233/91
// lv8 Yonakuni: 8/215/110
// lv8 Okinotori: 8/224/113
// lv8 Minamitori: 8/237/110
static const uint32_t kJapanLeft8 = 214;
static const uint32_t kJapanRight8 = 237;
static const uint32_t kJapanTop8 = 90;
static const uint32_t kJapanBottom8 = 114;

void createJapanTiles(int32_t aMinZoomLevel, const HimawariStandardData& aData,
                      DataType aType,
                      uint32_t aBand0, uint32_t aBand1, uint32_t aBand2) {
    if (aMinZoomLevel > 7) {
        return;
    }

    qDebug() << "The status of band" << aBand0 << " is: " << aData.mBands[aBand0].hasData();
    qDebug() << "The status of band" << aBand1 << " is: " << aData.mBands[aBand1].hasData();
    qDebug() << "The status of band" << aBand2 << " is: " << aData.mBands[aBand2].hasData();

    if (!aData.mBands[aBand0].hasData() ||
            !aData.mBands[aBand1].hasData() ||
            !aData.mBands[aBand2].hasData()) {
        return;
    }

    uint32_t left = kJapanLeft8;
    uint32_t right = kJapanRight8;
    uint32_t top = kJapanTop8;
    uint32_t bottom = kJapanBottom8;

    int32_t z = 8;

    for (uint32_t x = left; x <= right; x++) {
        for (uint32_t y = top; y < bottom; y++) {
            createTile(z, x, y, aData, aType, aBand0, aBand1, aBand2);
        }
    }
}

void createJapanTiles(int32_t aMinZoomLevel, const HimawariStandardData& aData,
                      DataType aType,
                      uint32_t aBand0, uint32_t aBand1) {
    if (aMinZoomLevel > 7) {
        return;
    }

    if (!aData.mBands[aBand0].hasData() ||
            !aData.mBands[aBand1].hasData()) {
        return;
    }

    uint32_t left = kJapanLeft8;
    uint32_t right = kJapanRight8;
    uint32_t top = kJapanTop8;
    uint32_t bottom = kJapanBottom8;

    int32_t z = 8;
    for (uint32_t x = left; x <= right; x++) {
        for (uint32_t y = top; y < bottom; y++) {
            createTile(z, x, y, aData, aType, aBand0, aBand1);
        }
    }
}

void createJapanTiles(int32_t aMinZoomLevel, const HimawariStandardData& aData,
                      DataType aType,
                      uint32_t aBand) {
    if (aMinZoomLevel > 7) {
        return;
    }

    if (!aData.mBands[aBand].hasData()) {
        return;
    }

    uint32_t left = kJapanLeft8;
    uint32_t right = kJapanRight8;
    uint32_t top = kJapanTop8;
    uint32_t bottom = kJapanBottom8;

    int32_t z = 8;
    for (uint32_t x = left; x <= right; x++) {
        for (uint32_t y = top; y < bottom; y++) {
            createTile(z, x, y, aData, aType, aBand);
        }
    }
}

} // hsd2tms

template <typename T>
void printDuration(const char* aTitle, const T& aStart, const T& aEnd) {
    std::cout << "Duration (" << aTitle << "): "
              << double(std::chrono::duration_cast
                        <std::chrono::microseconds>(aEnd - aStart).count() / 1000)
                 / 1000.
              << " seconds ("
              << (std::chrono::duration_cast
                  <std::chrono::seconds>(aEnd - aStart).count() / 60)
              << " minutes)\n";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto t0 = std::chrono::system_clock::now();

//    hsd2tms::HimawariStandardData himawariData(argc - 1);
//    for (int i = 1; i < argc; i++) {
//        himawariData.append(argv[i]);
//    }
    hsd2tms::HimawariStandardData himawariData(argc - 1);
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B01_JP02_R10_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B02_JP02_R10_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B03_JP02_R05_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B04_JP02_R10_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B05_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B06_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B07_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B08_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B09_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B10_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B11_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B12_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B13_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B14_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B15_JP02_R20_S0101.DAT");
    himawariData.append("E://02-QtProject//HSD2TMS//data//HS_H08_20150125_0230_JP02//HS_H08_20150125_0230_B16_JP02_R20_S0101.DAT");
//    for (int i = 1; i < argc; i++) {
//        himawariData.append(argv[i]);
//    }
    himawariData.sort();
    auto t1 = std::chrono::system_clock::now();

    hsd2tms::createJapanTiles(0, himawariData, hsd2tms::TypeRadiation, 0, 1, 2);
    hsd2tms::createTiles(8, himawariData, hsd2tms::TypeRadiation, 0, 1, 2);
    hsd2tms::createTiles(8, himawariData, hsd2tms::TypeTemperature, 14);
    hsd2tms::createJapanTiles(0, himawariData, hsd2tms::TypeDust, 12, 13);
    hsd2tms::createJapanTiles(0, himawariData, hsd2tms::TypeRadiation, 1);
    hsd2tms::createJapanTiles(0, himawariData, hsd2tms::TypeTemperature, 14);
    hsd2tms::createTiles(6, himawariData, hsd2tms::TypeRadiation, 3, 4, 5);
    hsd2tms::createTiles(5, himawariData, hsd2tms::TypeTemperature, 14);

    auto t2 = std::chrono::system_clock::now();
    hsd2tms::colorchart();
    printDuration("Load Data", t0, t1);
    printDuration("Create Files", t1, t2);

    return a.exec();
}
