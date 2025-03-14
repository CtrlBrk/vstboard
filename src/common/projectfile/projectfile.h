/**************************************************************************
#    Copyright 2010-2025 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef PROJECTFILE_H
#define PROJECTFILE_H

//#include "../precomp.h"

#define PROJECT_FILE_KEY 0x757b0a5d
#define SETUP_FILE_KEY 0x7575e711
#define SETUPANDPROJECT_FILE_KEY 0x75751337

class MainHost;
class ProjectFile : public QObject
{
Q_OBJECT
public:
    static bool SaveToProjectFile(MainHost *myHost,QString filePath);
    static bool SaveToSetupFile(MainHost *myHost,QString filePath);
    static bool LoadFromFile(MainHost *myHost,QString filePath);
    static bool ToStream(MainHost *myHost,QDataStream &out, quint32 fileKey=SETUPANDPROJECT_FILE_KEY);
    static bool FromStream(MainHost *myHost,QDataStream &in);
    static bool SaveChunk(const QString &chunkName, QByteArray &ba, QDataStream &out);
    static bool LoadNextChunk( QString &chunkName, QByteArray &ba, QDataStream &in);
};

#endif // PROJECTFILE_H

