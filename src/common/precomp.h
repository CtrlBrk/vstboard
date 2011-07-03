/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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
//#ifndef PRECOMP_H
//#define PRECOMP_H

#if defined __cplusplus

    #define QT_USE_FAST_CONCATENATION
    #define QT_USE_FAST_OPERATOR_PLUS

    #define VST_FORCE_DEPRECATED 0

    #ifdef WIN32
        #define NOMINMAX
        #include "Windows.h"
    #endif

    #include <string>
    #include <cmath>
    #include <algorithm>

    #include <QAbstractItemView>
    #include <QtAlgorithms>
    #include <QtGui/QApplication>
    #include <QDialog>
    #include <QDir>
    #include <QDragEnterEvent>
    #include <QDropEvent>
    #include <QFileDialog>
    #include <QFileSystemModel>
    #include <QFocusEvent>
    #include <QFrame>
    #include <QGraphicsBlurEffect>
    #include <QGraphicsGridLayout>
    #include <QGraphicsLinearLayout>
    #include <QGraphicsLineItem>
    #include <QGraphicsRectItem>
    #include <QGraphicsScene>
    #include <QGraphicsSceneDragDropEvent>
    #include <QGraphicsSceneMouseEvent>
    #include <QGraphicsTextItem>
    #include <QGraphicsView>
    #include <QGraphicsWidget>
    #include <QHBoxLayout>
    #include <QLibrary>
    #include <QLineEdit>
    #include <QListWidgetItem>
    #include <QMainWindow>
    #include <QMap>
    #include <qmath.h>
    #include <QMenu>
    #include <QMessageBox>
    #include <QMimeData>
    #include <QMultiHash>
    #include <QMutex>
    #include <QObject>
    #include <QReadWriteLock>
    #include <QScrollBar>
    #include <QSemaphore>
    #include <QSettings>
    #include <QSharedData>
    #include <QSharedPointer>
    #include <QSignalMapper>
    #include <QStandardItemModel>
    #include <QStringList>
    #include <QStyledItemDelegate>
    #include <QThread>
    #include <QTime>
    #include <QTimer>
    #include <QTreeWidget>
    #include <QUrl>
    #include <QWidget>

    #ifdef SCRIPTENGINE
        #include <QtScript>
    #endif

    #ifdef QT_NO_DEBUG
        #define debug(...)
        #define debug2(...)
    #else
        #include <QDebug>
        #define debug(...) {qDebug(__VA_ARGS__);}
        #define debug2(...) {qDebug()__VA_ARGS__;}
    #endif

#endif
//#endif // PRECOMP_H
