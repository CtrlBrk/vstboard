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

//#include "precomp.h"
#include "keybind.h"
#include "keybindingdialog.h"
#include "ui_keybindingdialog.h"
#include "buttonswidget.h"
#include "modifierswidget.h"
#include "keypressedwidget.h"

KeyPressedDelegate::KeyPressedDelegate(QObject *parent) : QStyledItemDelegate (parent)
{
}

QWidget *KeyPressedDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    KeypressedWidget *editor = new KeypressedWidget(parent);

    return editor;
}

//mode type delegate

MoveTypeDelegate::MoveTypeDelegate(QObject *parent) : QStyledItemDelegate (parent)
{
}

QWidget *MoveTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QComboBox *editor = new QComboBox(parent);
    QMetaEnum moveInput =  KeyBind::staticMetaObject.enumerator( KeyBind::staticMetaObject.indexOfEnumerator("MoveInputs") );
    for(int i=0; i<moveInput.keyCount(); ++i) {
        editor->addItem(moveInput.key(i),moveInput.value(i));
    }

    return editor;
}
void MoveTypeDelegate::setEditorData(QWidget *editor,const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox*>(editor);
    combo->setCurrentIndex( combo->findText(index.model()->data(index, Qt::EditRole).toString()) );
}
void MoveTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox*>(editor);
    model->setData(index, combo->currentText(), Qt::EditRole);
    model->setData(index, combo->itemData(combo->currentIndex()), Qt::UserRole+1);
}
void MoveTypeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//mouse buttons delegate

MouseButtonsDelegate::MouseButtonsDelegate(QObject *parent) : QStyledItemDelegate (parent)
{
}

void MouseButtonsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString str;
    int mb = index.data().toInt();
    if(mb & Qt::LeftButton)
        str.append("Left ");
    if(mb & Qt::MiddleButton)
        str.append("Mid ");
    if(mb & Qt::RightButton)
        str.append("Right ");
    if(mb & Qt::XButton1)
        str.append("4 ");
    if(mb & Qt::XButton2)
        str.append("5");
    painter->drawText(option.rect,str);
}

QSize MouseButtonsDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *MouseButtonsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    ButtonsWidget *editor = new ButtonsWidget(parent);
    return editor;
}
void MouseButtonsDelegate::setEditorData(QWidget *editor,const QModelIndex &index) const
{
    ButtonsWidget *combo = static_cast<ButtonsWidget*>(editor);
    combo->SetData(index.data(Qt::EditRole).toInt());
}
void MouseButtonsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    ButtonsWidget *combo = static_cast<ButtonsWidget*>(editor);
    model->setData(index, combo->GetData(), Qt::EditRole);
}
void MouseButtonsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//modifier delegate

ModifierDelegate::ModifierDelegate(QObject *parent) : QStyledItemDelegate (parent)
{
}

void ModifierDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString str;
    int mb = index.data().toInt();
    if(mb & Qt::ShiftModifier)
        str.append("Shift ");
    if(mb & Qt::ControlModifier)
        str.append("Ctrl ");
    if(mb & Qt::AltModifier)
        str.append("Alt");
    painter->drawText(option.rect,str);
}

QSize ModifierDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *ModifierDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    ModifiersWidget *editor = new ModifiersWidget(parent);
    return editor;
}
void ModifierDelegate::setEditorData(QWidget *editor,const QModelIndex &index) const
{
    ModifiersWidget *combo = static_cast<ModifiersWidget*>(editor);
    combo->SetData(index.data(Qt::EditRole).toInt());
}
void ModifierDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    ModifiersWidget *combo = static_cast<ModifiersWidget*>(editor);
    model->setData(index, combo->GetData(), Qt::EditRole);
}
void ModifierDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


KeyBindingDialog::KeyBindingDialog(KeyBind *bind, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyBindingDialog),
    bind(bind),
    modelMain(0),
    modelModes(0)
{
    ui->setupUi(this);
    Init();
}

void KeyBindingDialog::Init()
{
    modelMain = bind->GetMainBindingModel();
    ui->tableMain->setModel(modelMain);
    ui->tableMain->resizeColumnsToContents();
    ui->tableMain->setItemDelegateForColumn(1,&keyPressedDelegate);

    modelModes = bind->GetModesModel();
    ui->listModes->setModel(modelModes);
    ui->listModes->setCurrentIndex(modelModes->index(0,0));
    ui->tableMode->setModel(modelModes);
    ui->tableMode->setRootIndex(modelModes->index(0,0));
    ui->tableMode->setItemDelegateForColumn(1,&moveDelegate);
    ui->tableMode->setItemDelegateForColumn(2,&buttonsDelegate);
    ui->tableMode->setItemDelegateForColumn(3,&modDelegate);
    ui->tableMode->resizeColumnsToContents();
}

KeyBindingDialog::~KeyBindingDialog()
{
    delete ui;
}

void KeyBindingDialog::accept()
{
    bind->SetMainBindingModel(modelMain);
    bind->SetModesModel(modelModes);
    bind->SaveInRegistry();
    QDialog::accept();
}

void KeyBindingDialog::on_listModes_clicked(const QModelIndex &index)
{
    ui->tableMode->setModel(modelModes);
    ui->tableMode->setRootIndex(index);
}


void KeyBindingDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(button == ui->buttonBox->button( QDialogButtonBox::RestoreDefaults)) {
        QMessageBox msg(QMessageBox::Warning,tr("Restore default"),tr("Restore default mapping ?"),QMessageBox::Ok|QMessageBox::Cancel);
        msg.exec();
        if(msg.result()!=QMessageBox::Ok)
            return;

        bind->SetDefaults();
        Init();
    }
}
