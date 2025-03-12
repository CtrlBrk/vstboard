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
    colorGroupNames.insert( ColorGroups::VstPlugin32, tr("Vst32 Plugin") );
    colorGroupNames.insert( ColorGroups::Vst3Plugin, tr("Vst3 Plugin") );
    colorGroupNames.insert( ColorGroups::ClapPlugin, tr("Clap Plugin") );
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

    InitPresets("default");
}

void ViewConfig::InitPresets(const QString &preset)
{
    /*
    AddColor(preset,ColorGroups::Programs,Colors::HighlightBackground,QColor(170,255,100,255));

    AddColor(preset,ColorGroups::VstPlugin,Colors::Background,QColor(255,255,128,128));
    AddColor(preset,ColorGroups::Vst3Plugin,Colors::Background,QColor(255,128,255,128));
    AddColor(preset,ColorGroups::ClapPlugin,Colors::Background,QColor(128,255,255,128));
    AddColor(preset,ColorGroups::VstPlugin32,Colors::Background,QColor(255,128,128,128));

    AddColor(preset,ColorGroups::AudioPin,Colors::Background,QColor(200,170,160,255));
    AddColor(preset,ColorGroups::AudioPin,Colors::VuMeter,QColor(210,210,100,255));

    AddColor(preset,ColorGroups::MidiPin,Colors::Background,QColor(190,230,230,255));
    AddColor(preset,ColorGroups::MidiPin,Colors::VuMeter,QColor(210,210,100,255));

    AddColor(preset,ColorGroups::ParameterPin,Colors::Background,QColor(160,185,200,255));
    AddColor(preset,ColorGroups::ParameterPin,Colors::VuMeter,QColor(210,210,100,255));

    AddColor(preset,ColorGroups::Cursor,Colors::Background,QColor(0,0,0,90));
    AddColor(preset,ColorGroups::Cursor,Colors::HighlightBackground,QColor(0,0,0,255));

    AddColor(preset,ColorGroups::Bridge,Colors::Background,QColor(160,170,160,255));
    AddColor(preset,ColorGroups::Bridge,Colors::Lines,QColor(0,0,0,255));

    AddColor(preset,ColorGroups::Object,Colors::Background,QColor(120,160,185,128));
    AddColor(preset,ColorGroups::Object,Colors::Text,QColor(0,0,0,255));
    AddColor(preset,ColorGroups::Object,Colors::HighlightBackground,QColor(255,255,0,127));

    AddColor(preset,ColorGroups::Panel,Colors::Lines,QColor(0,0,0,150));
    AddColor(preset,ColorGroups::Panel,Colors::Background,QColor(0,0,0,0));
    AddColor(preset,ColorGroups::Panel,Colors::HighlightBackground,QColor(180,180,180,255));

    AddColor(preset,ColorGroups::Window,Colors::Text,QColor(0,0,0,255));
    AddColor(preset,ColorGroups::Window,Colors::Window,QColor(175,165,135,255));
    AddColor(preset,ColorGroups::Window,Colors::WindowText,QColor(0,0,0,255));
    AddColor(preset,ColorGroups::Window,Colors::Button,QColor(175,165,135,255));
    AddColor(preset,ColorGroups::Window,Colors::ButtonText,QColor(0,0,0,255));
    AddColor(preset,ColorGroups::Window,Colors::Base,QColor(190,190,190,255));

    listPresetsInSetup[preset]=listPresets[preset];
*/
    QString data("{\"presets\":[{\"groups\":[{\"colors\":[{\"a\":255,\"b\":45,\"g\":43,\"id\":1,\"r\":44},{\"a\":255,\"b\":90,\"g\":87,\"id\":2,\"r\":88},{\"a\":255,\"b\":218,\"g\":218,\"id\":6,\"r\":218},{\"a\":255,\"b\":75,\"g\":75,\"id\":7,\"r\":77},{\"a\":255,\"b\":89,\"g\":89,\"id\":9,\"r\":90},{\"a\":255,\"b\":196,\"g\":196,\"id\":10,\"r\":197}],\"id\":1},{\"colors\":[{\"a\":50,\"b\":8,\"g\":8,\"id\":3,\"r\":8},{\"a\":197,\"b\":225,\"g\":225,\"id\":4,\"r\":225},{\"a\":179,\"b\":255,\"g\":255,\"id\":8,\"r\":255}],\"id\":2},{\"colors\":[{\"a\":0,\"b\":0,\"g\":0,\"id\":3,\"r\":0},{\"a\":255,\"b\":181,\"g\":181,\"id\":8,\"r\":181}],\"id\":3},{\"colors\":[{\"a\":200,\"b\":158,\"g\":158,\"id\":3,\"r\":158},{\"a\":127,\"b\":0,\"g\":255,\"id\":4,\"r\":255},{\"a\":255,\"b\":0,\"g\":0,\"id\":7,\"r\":0}],\"id\":4},{\"colors\":[{\"a\":205,\"b\":116,\"g\":234,\"id\":3,\"r\":180}],\"id\":5},{\"colors\":[{\"a\":253,\"b\":143,\"g\":183,\"id\":3,\"r\":210},{\"a\":255,\"b\":100,\"g\":210,\"id\":5,\"r\":210}],\"id\":6},{\"colors\":[{\"a\":255,\"b\":249,\"g\":215,\"id\":3,\"r\":195},{\"a\":255,\"b\":100,\"g\":210,\"id\":5,\"r\":210}],\"id\":7},{\"colors\":[{\"a\":255,\"b\":147,\"g\":244,\"id\":3,\"r\":142},{\"a\":255,\"b\":100,\"g\":210,\"id\":5,\"r\":210}],\"id\":8},{\"colors\":[{\"a\":90,\"b\":0,\"g\":0,\"id\":3,\"r\":0},{\"a\":255,\"b\":0,\"g\":0,\"id\":4,\"r\":0}],\"id\":9},{\"colors\":[{\"a\":255,\"b\":23,\"g\":123,\"id\":4,\"r\":42}],\"id\":10},{\"colors\":[{\"a\":255,\"b\":0,\"g\":0,\"id\":12,\"r\":0}],\"id\":11},{\"colors\":[{\"a\":128,\"b\":117,\"g\":63,\"id\":3,\"r\":117}],\"id\":12},{\"colors\":[{\"a\":128,\"b\":255,\"g\":255,\"id\":3,\"r\":128}],\"id\":13},{\"colors\":[{\"a\":128,\"b\":111,\"g\":111,\"id\":3,\"r\":181}],\"id\":14}],\"name\":\"Dark\"},{\"groups\":[{\"colors\":[{\"a\":255,\"b\":229,\"g\":229,\"id\":1,\"r\":231},{\"a\":255,\"b\":191,\"g\":191,\"id\":2,\"r\":191},"
                 "{\"a\":255,\"b\":30,\"g\":30,\"id\":6,\"r\":30},"
                 "{\"a\":155,\"b\":155,\"g\":155,\"id\":7,\"r\":255},"
                 "{\"a\":255,\"b\":200,\"g\":200,\"id\":9,\"r\":200},{\"a\":255,\"b\":50,\"g\":50,\"id\":10,\"r\":50}],\"id\":1},{\"colors\":[{\"a\":78,\"b\":149,\"g\":149,\"id\":3,\"r\":149},{\"a\":197,\"b\":225,\"g\":225,\"id\":4,\"r\":225},{\"a\":179,\"b\":0,\"g\":0,\"id\":8,\"r\":0}],\"id\":2},{\"colors\":[{\"a\":0,\"b\":255,\"g\":255,\"id\":3,\"r\":255},{\"a\":255,\"b\":50,\"g\":50,\"id\":8,\"r\":50}],\"id\":3},{\"colors\":[{\"a\":200,\"b\":158,\"g\":158,\"id\":3,\"r\":158},{\"a\":127,\"b\":0,\"g\":255,\"id\":4,\"r\":255},{\"a\":255,\"b\":0,\"g\":0,\"id\":7,\"r\":0}],\"id\":4},{\"colors\":[{\"a\":205,\"b\":116,\"g\":234,\"id\":3,\"r\":180}],\"id\":5},{\"colors\":[{\"a\":253,\"b\":143,\"g\":183,\"id\":3,\"r\":210},{\"a\":255,\"b\":100,\"g\":210,\"id\":5,\"r\":210}],\"id\":6},{\"colors\":[{\"a\":255,\"b\":249,\"g\":215,\"id\":3,\"r\":195},{\"a\":255,\"b\":100,\"g\":210,\"id\":5,\"r\":210}],\"id\":7},{\"colors\":[{\"a\":255,\"b\":147,\"g\":244,\"id\":3,\"r\":142},{\"a\":255,\"b\":100,\"g\":210,\"id\":5,\"r\":210}],\"id\":8},{\"colors\":[{\"a\":90,\"b\":0,\"g\":0,\"id\":3,\"r\":0},{\"a\":255,\"b\":0,\"g\":0,\"id\":4,\"r\":0}],\"id\":9},{\"colors\":[{\"a\":255,\"b\":54,\"g\":255,\"id\":4,\"r\":91}],\"id\":10},{\"colors\":[{\"a\":255,\"b\":0,\"g\":0,\"id\":11,\"r\":0}],\"id\":11},{\"colors\":[{\"a\":128,\"b\":117,\"g\":63,\"id\":3,\"r\":117}],\"id\":12},{\"colors\":[{\"a\":128,\"b\":255,\"g\":255,\"id\":3,\"r\":128}],\"id\":13},{\"colors\":[{\"a\":128,\"b\":111,\"g\":111,\"id\":3,\"r\":181}],\"id\":14}],\"name\":\"Default\"}]}");
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject obj = doc.object();
    fromJson(obj);
}

/*!
  Translate the first row of keys (1,2,3...) into a float number
    \param key code
    \returns value from 0 to 1.0f or -1.0f if key not found
    */
float ViewConfig::KeyboardNumber(int key)
{
    if(key==Qt::Key_Ampersand   || key==Qt::Key_1) return 0.0f;
    if(key==Qt::Key_Eacute      || key==Qt::Key_2) return 0.1f;
    if(key==Qt::Key_QuoteDbl    || key==Qt::Key_3) return 0.2f;
    if(key==Qt::Key_Apostrophe  || key==Qt::Key_4) return 0.3f;
    if(key==Qt::Key_ParenLeft   || key==Qt::Key_5) return 0.4f;
    if(key==Qt::Key_Minus       || key==Qt::Key_6) return 0.5f;
    if(key==Qt::Key_Egrave      || key==Qt::Key_7) return 0.6f;
    if(key==Qt::Key_Underscore  || key==Qt::Key_8) return 0.7f;
    if(key==Qt::Key_Ccedilla    || key==Qt::Key_9) return 0.8f;
    if(key==Qt::Key_Agrave      || key==Qt::Key_0) return 0.9f;
    if(key==Qt::Key_ParenRight  || key==Qt::Key_Equal) return 1.0f;
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
    //viewConfigPresetList * l = GetListOfPresets();
    //l->value(preset)[groupId][colorId]=color;
    //(*GetListOfPresets())[preset][groupId][colorId]=color;

    listPresets[preset][groupId][colorId]=color;
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
        "   background: " + GetColor(ColorGroups::Window,Colors::Base).name() + ";"
        "}"
        "QScrollBar {"
        "   color: " + GetColor(ColorGroups::Window,Colors::Button).name() + ";"
        "   background-color: " + GetColor(ColorGroups::Window,Colors::Window).name() + ";"
        "}"
        "QScrollBar::handle:vertical {"
        "   border: 1px solid " + GetColor(ColorGroups::Window,Colors::ButtonText).name() + ";"
        "}"
        "QScrollBar::sub-line:vertical,QScrollBar::add-line:vertical {"
        "   border: 0;"
        "}"
        "QListView, QSplitter {"
        "   border: 0;"
        "}"
        "QSplitter::handle {"
        "   background-color: " + GetColor(ColorGroups::Window,Colors::Base).name() + ";"
        "}"
        "QPushButton {                    "
        "    background-color:" + GetColor(ColorGroups::Window,Colors::Button).name() + ";    "
        "    color: " + GetColor(ColorGroups::Window,Colors::ButtonText).name() + ";               "
        "}                                "
        "                                 "
        "QPushButton:hover:!pressed {     "
        "    background-color: " + GetColor(ColorGroups::Window,Colors::Window).name() + ";    "
        "}                                "
        "                                 "
        "QPushButton:pressed {           "
        "    background-color: " + GetColor(ColorGroups::Window,Colors::WindowText).name() + ";    "
        "}                                "
        "                                 "
        "QPushButton:disabled {          "
        "    background-color: " + GetColor(ColorGroups::Window,Colors::Window).name() + ";    "
        "}                                "
        "QListView::item:selected {"
        "   background-color: " + GetColor(ColorGroups::Window,Colors::WindowText).name() + ";    "
        "}"
        "QTreeView::item:selected {"
        "   background-color: " + GetColor(ColorGroups::Window,Colors::WindowText).name() + ";    "
        "}"
        "QTableView {"
        "   selection-background-color: " + GetColor(ColorGroups::Window,Colors::WindowText).name() + ";    "
        "}"
    );
//    LOG(st);
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

#ifndef QT_NO_DEBUG
    QJsonObject json;
    toJson(json);
    LOG(json);
#endif
}

void ViewConfig::LoadFromRegistry()
{
    SetSavedInSetup(false);
    QVariant lVar = settings->GetSetting("Colors", 0);
    if( lVar==0 ) {
        SaveInRegistry();
        lVar = settings->GetSetting("Colors");
    }
    QByteArray tmpBa( lVar.value<QByteArray>() );
    QDataStream tmpStream( &tmpBa , QIODevice::ReadWrite);
    fromStream( tmpStream );
    QString name = settings->GetSetting("ColorPreset", "Default").toString();
    LoadPreset(name);
}

void ViewConfig::toJson(QJsonObject &json) const
{
    viewConfigPresetList tmpPresets;
    if(savedInSetupFile)
        tmpPresets=listPresetsInSetup;
    else
        tmpPresets=listPresets;

    QJsonArray presetArray;
    viewConfigPresetList::const_iterator ip = tmpPresets.constBegin();
    while(ip!=tmpPresets.constEnd()) {
        QJsonObject jPreset;
        jPreset["name"] = ip.key();

        QJsonArray grpArray;
        viewConfigPreset::const_iterator i = ip.value().constBegin();
        while(i!=ip.value().constEnd()) {
            QJsonObject jGrp;
            jGrp["id"] = i.key();

            QJsonArray colArray;
            QMap<Colors::Enum,QColor>::const_iterator j = i.value().begin();
            while(j!=i.value().end()) {
                QJsonObject jCol;
                jCol["id"] = j.key();
                jCol["r"] = j.value().red();
                jCol["g"] = j.value().green();
                jCol["b"] = j.value().blue();
                jCol["a"] = j.value().alpha();
                colArray.append(jCol);
                ++j;
            }
            jGrp["colors"] = colArray;
            grpArray.append(jGrp);
            ++i;
        }
        jPreset["groups"] = grpArray;

        presetArray.append(jPreset);
        ++ip;
    }
    json["presets"] = presetArray;
}

void ViewConfig::fromJson(QJsonObject &json)
{
    if(savedInSetupFile) {
        listPresetsInSetup.clear();
    } else {
        listPresets.clear();
    }

    QJsonArray presetArray = json["presets"].toArray();
    for (int i = 0; i < presetArray.size(); ++i) {
        QJsonObject jPreset = presetArray[i].toObject();
        QString presetName = jPreset["name"].toString();

        QJsonArray grpArray = jPreset["groups"].toArray();
        for (int j = 0; j < grpArray.size(); ++j) {
            QJsonObject jGrp = grpArray[j].toObject();
            ColorGroups::Enum grpId = static_cast<ColorGroups::Enum>(jGrp["id"].toInt());

            QJsonArray colArray = jGrp["colors"].toArray();
            for (int k = 0; k < colArray.size(); ++k) {
                QJsonObject jCol = colArray[k].toObject();
                Colors::Enum colId = static_cast<Colors::Enum>(jCol["id"].toInt());
                int r = jCol["r"].toInt();
                int g = jCol["g"].toInt();
                int b = jCol["b"].toInt();
                int a = jCol["a"].toInt();

                if(savedInSetupFile)
                    listPresetsInSetup[presetName][grpId][colId] = QColor( r,g,b,a );
                else {
                    listPresets[presetName][grpId][colId] = QColor( r,g,b,a );
                }
            }
        }
    }

    emit NewSetupLoaded();
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
