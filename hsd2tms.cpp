/* 
 *    Copyright (C) 2015 Torisugari <torisugari@gmail.com>
 *
 *     Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *     The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <fstream>
#include <direct.h>
#include "hsd2tms.h"
#include <QDir>
#include <QImage>
#include <QSize>
#include <QImageWriter>

namespace hsd2tms {

inline void composePath(std::string& aPath, 
                        uint32_t aZ, uint32_t aX, uint32_t aY, bool aMkdir) {
    QDir dir;
    if (aMkdir) {
        dir.mkdir(QString::fromStdString(aPath));
//        mkdir(aPath.c_str(), 0755);
    }

    aPath += std::to_string(aZ);
    if (aMkdir) {
        dir.mkdir(QString::fromStdString(aPath));
//        mkdir(aPath.c_str(), 0755);
    }

    aPath += "/";
    aPath += std::to_string(aX);
    if (aMkdir) {
        dir.mkdir(QString::fromStdString(aPath));
//        mkdir(aPath.c_str(), 0755);
    }

    aPath += "/";
    aPath += std::to_string(aY);
    aPath += ".png";
}

void colorchart() {
    QVector<QRgb> RGBVector = getTemperatureColorTable();
    QImage colorChart(256, 10, QImage::Format_Indexed8);
    colorChart.setColorTable(RGBVector);
    for(int i = 0; i < 256; i++)
    {
        for(int j = 0; j < 10; j++)
        {
            colorChart.setPixel(i,j,i);
        }
    }
    //Because the original png file is so big,so compress them here
    QImageWriter imageWriter("./thermo.png", "PNG");
    imageWriter.setCompression(1);
    imageWriter.write(colorChart);
}
static const double kMaxRadiation[16] = 
{300., 280., 260., 300., 40., 10., 1., 1.5,
 3., 3., 3., 3., 3., 3., 3., 3.};
#if 0
band: 0
championR: 596.924
band: 1
championR: 570.674
band: 2
championR: 526.357
band: 3
championR: 313.535
band: 4
championR: 42.1191
band: 5
championR: 10.1764
band: 6
championR: 0.981368
band: 7
championR: 1.44067
band: 8
championR: 2.85773
#endif

inline uint8_t normalizeRadiation(double aRadiation, uint32_t aBand) {
    if (aRadiation <= 0) {
        return 0;
    }
    const double& max = kMaxRadiation[(aBand & 0xF)];
    return (max < aRadiation)? 0xFF : uint8_t((aRadiation * 255.) / max);
}

inline uint8_t normalizeTemperature(double aTemperature) {
    static const double kMin = 273.15 - 40.;
    static const double kMax = 273.15 + 40.;
    static const double kRange = kMax - kMin;

    // clamp
    if (aTemperature < kMin) {
        return 0x00;
    }
    else if (kMax < aTemperature) {
        return 0xFF;
    }
    return uint8_t((aTemperature - kMin) * 255. / kRange);
}

uint8_t normalizedData(const HimawariStandardData& aData,
                       double aLongitude, double aLatitude,
                       DataType aType, uint32_t aBand) {
    double rawdata;
    switch(aType) {
    case TypeRadiation:
        rawdata = aData.mBands[aBand].radiationAt(aLongitude, aLatitude);
        return normalizeRadiation(rawdata, aBand);
        break;
    case TypeTemperature:
        rawdata = aData.mBands[aBand].temperatureAt(aLongitude, aLatitude);
        return normalizeTemperature(rawdata);
        break;
    default:
        break;
    }
    return 0;
}

uint8_t normalizedData(const HimawariStandardData& aData,
                       double aLongitude, double aLatitude,
                       DataType aType, uint32_t aBand0, uint32_t aBand1) {
    double rawdata0, rawdata1;
    switch(aType) {
    case TypeDust:
        rawdata0 = aData.mBands[aBand0].radiationAt(aLongitude, aLatitude);
        rawdata1 = aData.mBands[aBand1].radiationAt(aLongitude, aLatitude);
        return normalizeRadiation(rawdata0 - rawdata1, aBand0);
        break;
    default:
        break;
    }
    return 0;
}

void createTile(const std::string& aDir, uint32_t aZ, uint32_t aX, uint32_t aY,
                const HimawariStandardData& aData,
                DataType aType,
                uint32_t aBand) {
    TileMapService tile;
    tile.init(aZ, aX, aY);

    std::string path(aDir);
    composePath(path, aZ, aX, aY, true);
    std::cout << "Creating " << path << std::endl;
    // User defined color table
    QVector<QRgb> RGBVector;
    switch (aType) {
    case TypeTemperature:
        RGBVector = getTemperatureColorTable();
        break;
    default:
        RGBVector = getPinkColorTable();
        break;
    }

    QImage image(256, 256, QImage::Format_Indexed8);
    image.setColorTable(RGBVector);
    for(int i = 0; i < 256; i++)
    {
        double latitude = tile.latitude(i);
        for (int j = 0; j < 256; j++)
        {
            double longitude = tile.longitude(j);
            image.setPixel(j,i, normalizedData(aData, longitude, latitude, aType, aBand));
        }
    }
    //Because the original png file is so big,so compress them here
    QImageWriter imageWriter(QString::fromStdString(path), "PNG");
    imageWriter.setCompression(1);
    imageWriter.write(image);
//    image.save(QString::fromStdString(path), "PNG");
}

void createTile(const std::string& aDir, uint32_t aZ, uint32_t aX, uint32_t aY,
                const HimawariStandardData& aData,
                DataType aType,
                uint32_t aBand0, uint32_t aBand1) {
    TileMapService tile;
    tile.init(aZ, aX, aY);

    std::string path(aDir);
    composePath(path, aZ, aX, aY, true);
    std::cout << "Creating " << path << std::endl;

    // User defined color table
    QVector<QRgb> RGBVector;
    switch (aType) {
    case TypeDust:
        RGBVector = getYellowColorTable();
        break;
    default:
        RGBVector = getPinkColorTable();
        break;
    }

    QImage image(256, 256, QImage::Format_Indexed8);
    image.setColorTable(RGBVector);
    for(int i = 0; i < 256; i++)
    {
        double latitude = tile.latitude(i);
        for (int j = 0; j < 256; j++)
        {
            double longitude = tile.longitude(j);
            image.setPixel(j,i, normalizedData(aData, longitude, latitude, aType, aBand1));
        }
    }
    //Because the original png file is so big,so compress them here
    QImageWriter imageWriter(QString::fromStdString(path), "PNG");
    imageWriter.setCompression(1);
    imageWriter.write(image);
//    image.save(QString::fromStdString(path), "PNG");
}

void createTile(const std::string& aDir, uint32_t aZ, uint32_t aX, uint32_t aY,
                const HimawariStandardData& aData,
                DataType aType,
                uint32_t aBand0, uint32_t aBand1, uint32_t aBand2) {
    TileMapService tile;
    tile.init(aZ, aX, aY);

    std::string path(aDir);
    composePath(path, aZ, aX, aY, true);
    std::cout << "Creating " << path << std::endl;

    //修改为通过QImage操作图片
    QImage image(256,256, QImage::Format_RGB32);
    for(int i = 0; i < 256; i++)
    {
        double latitude = tile.latitude(i);
        for (int j = 0; j < 256; j++)
        {
            double longitude = tile.longitude(j);
            int r = normalizedData(aData, longitude, latitude, aType, aBand2);
            int g = normalizedData(aData, longitude, latitude, aType, aBand1);
            int b = normalizedData(aData, longitude, latitude, aType, aBand0);
            image.setPixel(j, i, qRgb(r,g,b));
        }
    }
    //Because the original png file is so big,so compress them here
    QImageWriter imageWriter(QString::fromStdString(path), "PNG");
    imageWriter.setCompression(1);
    imageWriter.write(image);
}

void createTile(uint32_t aZ, uint32_t aX, uint32_t aY,
                const HimawariStandardData& aData,
                DataType aType,
                uint32_t aBand) {
    std::string path;
    DirNameProvider::compose(path, aType, aBand);
    createTile(path, aZ, aX, aY, aData, aType, aBand);
}

void createTile(uint32_t aZ, uint32_t aX, uint32_t aY,
                const HimawariStandardData& aData,
                DataType aType,
                uint32_t aBand0, uint32_t aBand1) {
    std::string path;
    DirNameProvider::compose(path, aType, aBand0);
    createTile(path, aZ, aX, aY, aData, aType, aBand0, aBand1);
}

void createTile(uint32_t aZ, uint32_t aX, uint32_t aY,
                const HimawariStandardData& aData,
                DataType aType,
                uint32_t aBand0, uint32_t aBand1, uint32_t aBand2) {
    std::string path;
    DirNameProvider::compose(path, aType, aBand0, aBand1, aBand2);
    createTile(path, aZ, aX, aY, aData, aType, aBand0, aBand1, aBand2);
}

inline void composeJSPath(std::string& aPath, 
                          uint32_t aZ, uint32_t aX, uint32_t aY, bool aMkdir) {
    QDir dir;
    if (aMkdir) {
        dir.mkdir(QString::fromStdString(aPath));
    }

    aPath += std::to_string(aZ);
    if (aMkdir) {
        dir.mkdir(QString::fromStdString(aPath));
    }

    aPath += "/";
    aPath += std::to_string(aX);
    if (aMkdir) {
        dir.mkdir(QString::fromStdString(aPath));
    }

    aPath += "/";
    aPath += std::to_string(aY);
    aPath += ".js";
}

void createAltitudeFile(uint32_t aZ, uint32_t aX, uint32_t aY,
                        const HimawariStandardData& aData,
                        const CloudTopAltitude& aTable) {
    TileMapService tile;
    tile.init(aZ, aX, aY);

    std::string path("./rad010203/");
    composeJSPath(path, aZ, aX, aY, true);
    std::ofstream output(path);

    output << "var altitude = new Float64Array([\n";
    for (int i = 0; i < 256; i++) {
        double latitude = tile.latitude(i);
        for (int j = 0; j < 256; j++) {
            double longitude = tile.longitude(j);
            double tb11, tb12;
            aData.brightnessTemperaturesAt(longitude, latitude, tb11, tb12);
            double zenith = aData.mSegments[0].zenith(longitude, latitude) * 180. /M_PI;
            double altitude = aTable.linear(tb11, tb12, zenith);
            if (altitude < 0.) {
                altitude = 0.;
            }
            output << altitude << "," << std::endl;
        }
    }
    output << "]);\n";
}

QVector<QRgb> getTemperatureColorTable()
{
    QVector<QRgb> RGBVector;
    for(int i = 0; i < 256; i++)
    {
        int red   = (i * i / 0xFF);
        int green = 0xFF - ((i - 0x7f) * (i - 0x80) / 0x40);
        int blue  = ((0xFF - i) * (0xFF - i)/ 0xFF);
        RGBVector.append(qRgb(red,green,blue));
    }
    return RGBVector;
}

QVector<QRgb> getPinkColorTable()
{
    QVector<QRgb> RGBVector;
    for(int i = 0; i < 256; i++)
        RGBVector.append(qRgb(0xFF,0xCC,0xCC));

    return RGBVector;
}

QVector<QRgb> getYellowColorTable()
{
    QVector<QRgb> RGBVector;
    for(int i = 0; i < 256; i++)
        RGBVector.append(qRgb(0xFF,0xFF,0x00));

    return RGBVector;
}

} // hsd2tms
