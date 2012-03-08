/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#ifndef VIEWCONFIG_H
#define VIEWCONFIG_H

//#include "precomp.h"
#include "keybind.h"
#include "settings.h"

namespace ColorGroups {
    enum Enum {
        ND,
        Window,
        Panel,
        Bridge,
        Object,
        VstPlugin,
        AudioPin,
        MidiPin,
        ParameterPin,
        Cursor,
        Programs
    };
}

namespace Colors {
    enum Enum {
        ND,
        Window,
        Base,
        Background,
        HighlightBackground,
        VuMeter,
        WindowText,
        Text,
        Lines,
        Button,
        ButtonText
//            AlternateBase,
//            ToolTipBase,
//            ToolTipText,
//            BrightText,
    };
}

#define viewConfigPreset QMap<ColorGroups::Enum, QMap<Colors::Enum, QColor> >
#define viewConfigPresetList QMap<QString, QMap<ColorGroups::Enum, QMap<Colors::Enum, QColor> > >


namespace View {

//    class ColorGroup
//    {
//    public:
//        QMap<Colors::Enum,QColor>listColors;

//    };


    class ObjectView;
    class ViewConfig : public QObject
    {
    Q_OBJECT

    public:
        ViewConfig( Settings *settings,  QObject *parent=0 );

        void SetColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
        QColor GetColor(ColorGroups::Enum groupId, Colors::Enum colorId);
        QString GetColorGroupName(ColorGroups::Enum groupId);
        QString GetColorName(Colors::Enum colorId);
        QPalette::ColorRole GetPaletteRoleFromColor(Colors::Enum colorId);
        QPalette GetPaletteFromColorGroup(ColorGroups::Enum groupId, const QPalette &oriPalette);
        void SetListGroups(viewConfigPreset newList);

        void SaveToFile( QDataStream & out );
        void LoadFromFile( QDataStream & in );
        void SaveInRegistry();
        void LoadFromRegistry();


        void LoadPreset(const QString &presetName);
        inline const QString & GetPresetName() const {return currentPresetName;}

        inline viewConfigPreset * GetCurrentPreset()
        {
            return &(*GetListOfPresets())[currentPresetName];
        }

        inline viewConfigPresetList * GetListOfPresets()
        {
            if(savedInSetupFile)
                return &listPresetsInSetup;
            else
                return &listPresets;
        }

        bool IsSavedInSetup() {return savedInSetupFile;}
        void SetSavedInSetup(bool inSetup) {if(savedInSetupFile==inSetup) return; savedInSetupFile=inSetup; currentPresetName="Default";}

        void AddPreset(QString &presetName);
        void RemovePreset(const QString &presetName);
        void RenamePreset(const QString &oldName, QString &newName);
        void CopyPreset(const QString &presetName, QString &newName);

        QDataStream & toStream (QDataStream &) const;
        QDataStream & fromStream (QDataStream &);

        static float KeyboardNumber(int key);

        KeyBind *keyBinding;
        bool AutoOpenGui;
        Settings *settings;

        QTimer *timerFalloff;

    protected:
        ///list of presets in registry
        viewConfigPresetList listPresets;

        ///list of presets saved in setup file
        viewConfigPresetList listPresetsInSetup;

        bool savedInSetupFile;
        void InitPresets();
        void AddColor(const QString &preset, ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
        void UpdateAllWidgets();

        /// list of groups names
        QMap<ColorGroups::Enum,QString>colorGroupNames;

        /// list of colors names
        QMap<Colors::Enum,QString>colorsNames;

        QString currentPresetName;

    signals:
        /*!
            emited when a color changed
            \param groupId group Id
            \param colorId color Id
            \param color the new color
        */
        void ColorChanged(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);

        void NewSetupLoaded();
    };

}


#endif // VIEWCONFIG_H
