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
#include "viewconfig.h"

using namespace View;

/*!
  \class View::ViewConfig
  \brief manage the application appearance
  */

/*!
  Constructor
  */
ViewConfig::ViewConfig(Settings *settings, QObject *parent) :
    QObject(parent),
    keyBinding(new KeyBind(settings)),
    AutoOpenGui(false),
    settings(settings),
    savedInSetupFile(false),
    currentPresetName("Default")
{
    timerFalloff = new QTimer(this);
    timerFalloff->start(120);

    colorGroupNames.insert( ColorGroups::ND, tr("-undefined-") );
    colorGroupNames.insert( ColorGroups::Window, tr("Window") );
    colorGroupNames.insert( ColorGroups::Panel, tr("Panel") );
    colorGroupNames.insert( ColorGroups::Bridge, tr("Bridge") );
    colorGroupNames.insert( ColorGroups::Object, tr("Object") );
    colorGroupNames.insert( ColorGroups::VstPlugin, tr("Vst Plugin") );
    colorGroupNames.insert( ColorGroups::AudioPin, tr("Audio Pin") );
    colorGroupNames.insert( ColorGroups::MidiPin, tr("Midi Pin") );
    colorGroupNames.insert( ColorGroups::ParameterPin, tr("Parameter Pin") );
    colorGroupNames.insert( ColorGroups::Cursor, tr("Cursor") );
    colorGroupNames.insert( ColorGroups::Programs, tr("Programs") );

    colorsNames.insert( Colors::ND, tr("-undefined-") );
    colorsNames.insert( Colors::Window, tr("Window") );
    colorsNames.insert( Colors::Base, tr("Base") );
    colorsNames.insert( Colors::Background, tr("Background") );
    colorsNames.insert( Colors::HighlightBackground, tr("Highlight Background") );
    colorsNames.insert( Colors::VuMeter, tr("Vu-meter") );
    colorsNames.insert( Colors::WindowText, tr("Window Text") );
    colorsNames.insert( Colors::Text, tr("Text") );
    colorsNames.insert( Colors::Lines, tr("Lines") );
    colorsNames.insert( Colors::Button, tr("Button") );
    colorsNames.insert( Colors::ButtonText, tr("Button Text") );

//    colorsNames.insert( Colors::AlternateBase, tr("Alternate Base") );
//    colorsNames.insert( Colors::ToolTipBase, tr("ToolTip Base") );
//    colorsNames.insert( Colors::ToolTipText, tr("ToolTip Text") );
//    colorsNames.insert( Colors::BrightText, tr("BrightText") );

    InitPresets();
}

void ViewConfig::InitPresets()
{
    AddColor("Default",ColorGroups::Programs,Colors::HighlightBackground,QColor(170,255,100,255));

    AddColor("Default",ColorGroups::VstPlugin,Colors::Background,QColor(255,255,128,128));

    AddColor("Default",ColorGroups::AudioPin,Colors::Background,QColor(200,170,160,255));
    AddColor("Default",ColorGroups::AudioPin,Colors::VuMeter,QColor(210,210,100,255));

    AddColor("Default",ColorGroups::MidiPin,Colors::Background,QColor(190,230,230,255));
    AddColor("Default",ColorGroups::MidiPin,Colors::VuMeter,QColor(210,210,100,255));

    AddColor("Default",ColorGroups::ParameterPin,Colors::Background,QColor(160,185,200,255));
    AddColor("Default",ColorGroups::ParameterPin,Colors::VuMeter,QColor(210,210,100,255));

    AddColor("Default",ColorGroups::Cursor,Colors::Background,QColor(0,0,0,90));
    AddColor("Default",ColorGroups::Cursor,Colors::HighlightBackground,QColor(0,0,0,255));

    AddColor("Default",ColorGroups::Bridge,Colors::Background,QColor(160,170,160,255));
    AddColor("Default",ColorGroups::Bridge,Colors::Lines,QColor(0,0,0,255));

    AddColor("Default",ColorGroups::Object,Colors::Background,QColor(120,160,185,128));
    AddColor("Default",ColorGroups::Object,Colors::Text,QColor(0,0,0,255));
    AddColor("Default",ColorGroups::Object,Colors::HighlightBackground,QColor(255,255,0,127));

    AddColor("Default",ColorGroups::Panel,Colors::Lines,QColor(0,0,0,150));
    AddColor("Default",ColorGroups::Panel,Colors::Background,QColor(0,0,0,0));
    AddColor("Default",ColorGroups::Panel,Colors::HighlightBackground,QColor(180,180,180,255));

    AddColor("Default",ColorGroups::Window,Colors::Text,QColor(0,0,0,255));
    AddColor("Default",ColorGroups::Window,Colors::Window,QColor(175,165,135,255));
    AddColor("Default",ColorGroups::Window,Colors::WindowText,QColor(0,0,0,255));
    AddColor("Default",ColorGroups::Window,Colors::Button,QColor(175,165,135,255));
    AddColor("Default",ColorGroups::Window,Colors::ButtonText,QColor(0,0,0,255));
    AddColor("Default",ColorGroups::Window,Colors::Base,QColor(190,190,190,255));

    listPresetsInSetup["Default"]=listPresets["Default"];
}

/*!
  Translate the first row of keys (1,2,3...) into a float number
    \param key code
    \returns value from 0 to 1.0f or -1.0f if key not found
    */
float ViewConfig::KeyboardNumber(int key)
{
    if(key==QKeySequence(tr("!"))[0] || key==QKeySequence(tr("1"))[0]) return 0.0f;
    if(key==QKeySequence(tr("@"))[0] || key==QKeySequence(tr("2"))[0]) return 0.1f;
    if(key==QKeySequence(tr("#"))[0] || key==QKeySequence(tr("3"))[0]) return 0.2f;
    if(key==QKeySequence(tr("$"))[0] || key==QKeySequence(tr("4"))[0]) return 0.3f;
    if(key==QKeySequence(tr("%"))[0] || key==QKeySequence(tr("5"))[0]) return 0.4f;
    if(key==QKeySequence(tr("^"))[0] || key==QKeySequence(tr("6"))[0]) return 0.5f;
    if(key==QKeySequence(tr("&"))[0] || key==QKeySequence(tr("7"))[0]) return 0.6f;
    if(key==QKeySequence(tr("*"))[0] || key==QKeySequence(tr("8"))[0]) return 0.7f;
    if(key==QKeySequence(tr("("))[0] || key==QKeySequence(tr("9"))[0]) return 0.8f;
    if(key==QKeySequence(tr(")"))[0] || key==QKeySequence(tr("0"))[0]) return 0.9f;
    if(key==QKeySequence(tr("_"))[0] || key==QKeySequence(tr("-"))[0]) return 1.0f;
    return -1.0f;
}

void ViewConfig::LoadPreset(const QString &presetName)
{
    currentPresetName=presetName;
    UpdateAllWidgets();

    if(!savedInSetupFile)
        settings->SetSetting("ColorPreset",currentPresetName);
}

/*!
  Update all colors with a new list, used by the dialog when changes are discarded
  \param newList list of groups
  */
void ViewConfig::SetListGroups(viewConfigPreset newList)
{
    *GetCurrentPreset() = newList;
    UpdateAllWidgets();

}

/*!
  Sends all colors to all widgets
  */
void ViewConfig::UpdateAllWidgets()
{
    viewConfigPreset::iterator i = GetCurrentPreset()->begin();
    while( i!=GetCurrentPreset()->end() ) {
        QMap<Colors::Enum,QColor> grp = i.value();
        QMap<Colors::Enum,QColor>::iterator j = grp.begin();
        while( j!=grp.end() ) {
            emit ColorChanged( i.key(), j.key(), j.value() );
            ++j;
        }
        ++i;
    }
    emit StylesheetChanged();
}

/*!
  Get the name of a group, used by the dialog
  \param groupId the group Id
  \return group name
  */
QString ViewConfig::GetColorGroupName(ColorGroups::Enum groupId)
{
    if(!colorGroupNames.contains(groupId))
        return tr("-unknown-");
    return colorGroupNames.value(groupId);
}

/*!
  Get the name of a color, used by the dialog
  \param colorId the color Id
  \return color name
  */
QString ViewConfig::GetColorName(Colors::Enum colorId)
{
    if(!colorsNames.contains(colorId))
        return "-unknown-";
    return colorsNames.value(colorId);
}

/*!
  Get the ColorRole corresponding to a color Id
  \param colorId the color Id
  \return palette role
  */
QPalette::ColorRole ViewConfig::GetPaletteRoleFromColor(Colors::Enum colorId)
{
    switch(colorId) {
        case Colors::Window:
            return QPalette::Window;
        case Colors::Text:
            return QPalette::Text;
        case Colors::Button:
            return QPalette::Button;
        case Colors::ButtonText:
            return QPalette::ButtonText;
        case Colors::WindowText:
            return QPalette::WindowText;
        case Colors::Base:
            return QPalette::Base;
//        case Colors::AlternateBase:
//            return QPalette::AlternateBase;
//        case Colors::ToolTipBase:
//            return QPalette::ToolTipBase;
//        case Colors::ToolTipText:
//            return QPalette::ToolTipText;
//        case Colors::BrightText:
//            return QPalette::BrightText;
        default:
            return QPalette::NoRole;
    }

}

/*!
  Update the given palette with the group colors
  \param groupId the group Id
  \param oriPalette the palette to modify
  \return modified palette
  */
//QPalette ViewConfig::GetPaletteFromColorGroup(ColorGroups::Enum groupId, const QPalette &oriPalette)
//{
//    if(!GetCurrentPreset()->contains(groupId))
//        return oriPalette;

//    QMap<Colors::Enum,QColor> grp = GetCurrentPreset()->value(groupId);
//    QPalette pal(oriPalette);

//    QMap<Colors::Enum,QColor>::iterator i = grp.begin();
//    while(i != grp.end()) {
//        pal.setColor( GetPaletteRoleFromColor( i.key() ), i.value() );
//        ++i;
//    }
//    return pal;
//}

/*!
  Add a color to a group
  \param preset preset name
  \param groupId group Id
  \param colorId color Id
  \param color the color
  */
void ViewConfig::AddColor(const QString &preset, ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    (*GetListOfPresets())[preset][groupId][colorId]=color;
    emit ColorChanged(groupId,colorId,color);
}

void ViewConfig::SetTheme(Colors::Enum colorId) {
    (*GetCurrentPreset())[ColorGroups::Theme].clear();
    (*GetCurrentPreset())[ColorGroups::Theme].insert(colorId,QColor());
    emit ColorChanged(ColorGroups::Theme,colorId,QColor());
}

/*!
  Modifiy a color, don't create it if not found
  \param groupId group Id
  \param colorId color Id
  \param color the new color
  */
void ViewConfig::SetColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if( !GetCurrentPreset()->contains(groupId) || !GetCurrentPreset()->value(groupId).contains(colorId) )
        return;

    (*GetCurrentPreset())[groupId].insert(colorId,color);
    emit ColorChanged(groupId,colorId,color);

    if(groupId == ColorGroups::Window) {
        emit StylesheetChanged();
    }

}

/*!
  Get a color
  \param groupId group Id
  \param colorId color Id
  \return the color or a default color if not found
  */
QColor ViewConfig::GetColor(ColorGroups::Enum groupId, Colors::Enum colorId)
{
    if(!GetCurrentPreset()->contains(groupId))
        return QColor();

    return GetCurrentPreset()->value(groupId).value(colorId,QColor());
}

//QPalette ViewConfig::GetPalette(ColorGroups::Enum groupId) {
//    QPalette pal;

//    QMap<Colors::Enum,QColor> grp = (*GetCurrentPreset())[groupId];
//    QMap<Colors::Enum,QColor>::iterator i = grp.begin();
//    while( i!=grp.end() ) {
//        qDebug() << i.key();
//        qDebug() << i.value();
//        pal.setColor( GetPaletteRoleFromColor(i.key()), i.value() );
//        ++i;
//    }
//    return pal;
//}

QString ViewConfig::GetSyleSheet() {
//    foreach(auto c, GetCurrentPreset()[ColorGroups::Window]) {

//    }
    QString st(
        "QWidget {"
        "   background: " + GetColor(ColorGroups::Window,Colors::Window).name() + ";"
        "   color: " + GetColor(ColorGroups::Window,Colors::WindowText).name() + ";"
        "   selection-color: " + GetColor(ColorGroups::Window,Colors::Text).name() + ";"
        //"   outline: 2px solid red;"
        "}"
        "QDockWidget::title, QMainWindow  {"
        "   background:" + GetColor(ColorGroups::Window,Colors::Base).name() + ";"
        "}"
        "QScrollBar {"
        "   color: " + GetColor(ColorGroups::Window,Colors::ButtonText).name() + ";"
        "   background-color: " + GetColor(ColorGroups::Window,Colors::Button).name() + ";"
        "}"
        "QScrollBar::handle:vertical {"
        "   border: 1px solid " + GetColor(ColorGroups::Window,Colors::ButtonText).name() + ";"
        "}"
        "QScrollBar::sub-line:vertical,QScrollBar::add-line:vertical {"
        "   border: 0;"
        "}"
        "QListView, QSplitter {"
        "   border:0;"
        "}"
        "QSplitter::handle {"
        "   background-color: " + GetColor(ColorGroups::Window,Colors::Base).name() + ";"
        "}"
    );
//    qDebug() << st;
    return st;
}

void ViewConfig::AddPreset(QString &presetName)
{
    int count=2;
    QString newName=presetName;
    while(GetListOfPresets()->contains( newName )) {
        newName=presetName+" ("+QString::number(count)+")";
        count++;
    }
    GetListOfPresets()->insert( newName, (*GetListOfPresets())["Default"] );
    presetName=newName;
}

void ViewConfig::CopyPreset(const QString &presetName, QString &newName)
{
    int count=2;

    if(newName.isEmpty())
        newName=presetName;

    QString tmpNewName = newName;

    while(GetListOfPresets()->contains( tmpNewName )) {
        tmpNewName=newName+" ("+QString::number(count)+")";
        count++;
    }

    viewConfigPreset oldPreset;

    if(GetListOfPresets()->contains(presetName))
        oldPreset = (*GetListOfPresets())[presetName];
    else
        oldPreset = (*GetListOfPresets())["Default"];

    GetListOfPresets()->insert( tmpNewName, oldPreset );
    newName=tmpNewName;
}

void ViewConfig::RemovePreset(const QString &presetName)
{
    if(presetName=="Default" || !GetListOfPresets()->contains(presetName))
        return;
    GetListOfPresets()->remove(presetName);
}

void ViewConfig::RenamePreset( const QString &oldName, QString &newName)
{
    CopyPreset(oldName,newName);
    GetListOfPresets()->remove(oldName);
}

void ViewConfig::SaveToFile( QDataStream & out )
{
    bool lastMode = savedInSetupFile;
    QString lastPreset = currentPresetName;
    SetSavedInSetup(true);

    out << lastMode;
    toStream( out );
    out << currentPresetName;

    SetSavedInSetup(lastMode);
    currentPresetName=lastPreset;
}

void ViewConfig::LoadFromFile( QDataStream & in )
{
    SetSavedInSetup(true);
    bool newSavedInSetup;
    QString presetName;

    in >> newSavedInSetup;
    fromStream( in );
    in >> presetName;

    SetSavedInSetup(newSavedInSetup);

    if(savedInSetupFile)
        LoadPreset(presetName);
    else
        LoadFromRegistry();
}

void ViewConfig::SaveInRegistry()
{
    SetSavedInSetup(false);
    QByteArray tmpBa;
    QDataStream tmpStream( &tmpBa , QIODevice::ReadWrite);
    toStream( tmpStream );
    QVariant lVar=QVariant::fromValue(tmpBa);
    settings->SetSetting("Colors",lVar);
    settings->SetSetting("ColorPreset",currentPresetName);
}

void ViewConfig::LoadFromRegistry()
{
    SetSavedInSetup(false);
    QVariant lVar = settings->GetSetting("Colors", QVariant::Invalid);
    if( !lVar.isValid() ) {
        SaveInRegistry();
        lVar = settings->GetSetting("Colors");
    }
    QByteArray tmpBa( lVar.value<QByteArray>() );
    QDataStream tmpStream( &tmpBa , QIODevice::ReadWrite);
    fromStream( tmpStream );
    QString name = settings->GetSetting("ColorPreset", "Default").toString();
    LoadPreset(name);
}

/*!
  Put in a stream
  \param[in] out a QDataStream
  \return the stream
  */
QDataStream & ViewConfig::toStream(QDataStream & out) const
{
    viewConfigPresetList tmpPresets;
    if(savedInSetupFile)
        tmpPresets=listPresetsInSetup;
    else
        tmpPresets=listPresets;

    out << (quint16)tmpPresets.count();
    viewConfigPresetList::const_iterator ip = tmpPresets.constBegin();

    while(ip!=tmpPresets.constEnd()) {
        out << ip.key();
        out << (quint16)ip.value().count();

        viewConfigPreset::const_iterator i = ip.value().constBegin();
        while(i!=ip.value().constEnd()) {
            out << (quint8)i.key();
            out << (quint8)i.value().count();

            QMap<Colors::Enum,QColor>::const_iterator j = i.value().begin();
            while(j!=i.value().end()) {
                out << (quint8)j.key();
                out << (quint8)j.value().red();
                out << (quint8)j.value().green();
                out << (quint8)j.value().blue();
                out << (quint8)j.value().alpha();
                ++j;
            }
            ++i;
        }
        ++ip;
    }
    return out;
}

/*!
  Load from a stream
  \param[in] in a QDataStream
  \return the stream
  */
QDataStream & ViewConfig::fromStream(QDataStream & in)
{
    if(savedInSetupFile) {
        listPresetsInSetup.clear();
    } else {
        listPresets.clear();
    }

    quint16 nbPreset;
    in >> nbPreset;
    for(int p=0; p<nbPreset; p++) {
        QString presetName;
        in >> presetName;

        quint16 nbGrp;
        in >> nbGrp;

        for(int i=0; i<nbGrp; i++) {
            quint8 grpId;
            in >> grpId;
            quint8 nbCol;
            in >> nbCol;
            for(int j=0; j<nbCol; j++) {
                quint8 colId;
                in >> colId;

                quint8 red;
                in >> red;
                quint8 green;
                in >> green;
                quint8 blue;
                in >> blue;
                quint8 alpha;
                in >> alpha;

                if(savedInSetupFile)
                    listPresetsInSetup[presetName][(ColorGroups::Enum)grpId][(Colors::Enum)colId] = QColor( red,green,blue,alpha );
                else {
                    listPresets[presetName][(ColorGroups::Enum)grpId][(Colors::Enum)colId] = QColor( red,green,blue,alpha );
                }
            }
        }
    }

    emit NewSetupLoaded();
    return in;
}
