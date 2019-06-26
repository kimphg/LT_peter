/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QImage>
#include <QList>
#include <QThread>
#include <QDebug>
#include <QGuiApplication>
#include <qtconcurrentmap.h>
#include <QDir>
#include <QDirIterator>
int nFiles = 0;
QColor SeaColorGM(163,205,255,255);
QImage scale(const QImage &image)
{
    QImage tmp = image;
    for(int y = 0; y < tmp.height(); y++)
        for(int x= 0; x < tmp.width(); x++)
        {
            QColor color = tmp.pixelColor(x,y);
            int color_dist = abs(color.red()-163)+abs(color.green()-205)+abs(color.blue()-255);
            if(color_dist<100)
            {
                double ss=color_dist/100.0;
                tmp.setPixelColor(x,y,QColor(color.red()*ss,color.green()*ss,color.blue()*ss,255));
            }
        }
    return tmp;
}
void processDir(QString path)
{
    QDir    directory(path);
    QDirIterator directories(path, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while(directories.hasNext()){
            directories.next();
            processDir(directories.path()+"/"+directories.fileName());
        }
    QStringList imageNames = directory.entryList(QStringList() << "*.jpg" << "*.JPG",QDir::Files);
    foreach(QString imageName, imageNames) {
        QImage tmp(directory.absolutePath()+"/"+imageName);
        for(int y = 0; y < tmp.height(); y++)
            for(int x= 0; x < tmp.width(); x++)
            {
                QColor color = tmp.pixelColor(x,y);
                int color_dist = abs(color.red()-163)+abs(color.green()-205)+abs(color.blue()-255);
                if(color_dist<80)
                {
                    double ss=color_dist/80.0;
                    tmp.setPixelColor(x,y,QColor(color.red()*ss,color.green()*ss,color.blue()*ss,255));
                }
            }
        tmp.save(directory.absolutePath()+"/"+imageName);
        nFiles++;
        printf("\n%d",nFiles);
        //QList<QImage> images.append(QImage(imageName));
    }
}
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    //    QList<QImage> images;
//    processDir("D:/HR2D/mapData/GM/7/");
//    processDir("D:/HR2D/mapData/GM/8/");
    processDir("D:/HR2D/mapData/GM1/");
//    processDir("D:/HR2D/mapData/GM/12/");
    return 0;
}
