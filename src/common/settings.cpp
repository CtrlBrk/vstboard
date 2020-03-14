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

#include "settings.h"

Settings::Settings(const QString & settingsGroup, QObject *parent) :
    settingsGroup(settingsGroup),
    QObject(parent)
{
}


void Settings::SetSetting(QString name, QVariant value)
{
    settings.setValue(settingsGroup + name,value);
}

QVariant Settings::GetSetting(QString name, QVariant defaultVal)
{
    return settings.value(settingsGroup + name,defaultVal);
}

bool Settings::SettingDefined(QString name)
{
    return settings.contains(settingsGroup + name);
}
