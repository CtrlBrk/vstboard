/**************************************************************************
#    Copyright 2010-2020 Raphaël François
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

#ifndef KEYBINDINGDIALOG_H
#define KEYBINDINGDIALOG_H

//#include "precomp.h"

namespace Ui {
    class KeyBindingDialog;
}

class KeyPressedDelegate : public QStyledItemDelegate
 {
     Q_OBJECT

public:
    KeyPressedDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class MoveTypeDelegate : public QStyledItemDelegate
 {
     Q_OBJECT

 public:
     MoveTypeDelegate(QObject *parent = 0);
     QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
     void setEditorData(QWidget *editor, const QModelIndex &index) const;
     void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
     void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
 };

class MouseButtonsDelegate : public QStyledItemDelegate
 {
     Q_OBJECT

 public:
     MouseButtonsDelegate(QObject *parent = 0);
     void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
     QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
     QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
     void setEditorData(QWidget *editor, const QModelIndex &index) const;
     void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
     void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
 };

class ModifierDelegate : public QStyledItemDelegate
 {
     Q_OBJECT

 public:
     ModifierDelegate(QObject *parent = 0);
     void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
     QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
     QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
     void setEditorData(QWidget *editor, const QModelIndex &index) const;
     void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
     void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
 };

class KeyBind;
class KeyBindingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyBindingDialog(KeyBind *bind, QWidget *parent = 0);
    ~KeyBindingDialog();
private:
    void Init();

private slots:
    void on_listModes_clicked(const QModelIndex &index);

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::KeyBindingDialog *ui;
    KeyBind *bind;
    QStandardItemModel *modelMain;
    KeyPressedDelegate keyPressedDelegate;
    QStandardItemModel *modelModes;
    MoveTypeDelegate moveDelegate;
    MouseButtonsDelegate buttonsDelegate;
    ModifierDelegate modDelegate;

public slots:
    void accept();
};

#endif // KEYBINDINGDIALOG_H
