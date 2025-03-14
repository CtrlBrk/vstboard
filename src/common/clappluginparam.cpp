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
#include "clappluginparam.h"

// #include "plugin-host.hh"
#include "connectables/clapplugin.h"

ClapPluginParam::ClapPluginParam(Connectables::ClapPlugin &pluginHost, const clap_param_info &info, double value)
    : QObject(&pluginHost), _info(info), _value(value) {}

float ClapPluginParam::FloatFromValue() const {
    float val = (_value - _info.min_value) / (_info.max_value - _info.min_value);
    val = std::min(val,1.f);
    val = std::max(val,.0f);
    return val;
}

double ClapPluginParam::ValueFromFloat(float val) const {
    return val * (_info.max_value - _info.min_value) + _info.min_value;
}

void ClapPluginParam::setValue(double v) {
    if (_value == v)
        return;

    _value = v;
    emit valueChanged();
}

void ClapPluginParam::setModulation(double v) {
    if (_modulation == v)
        return;

    _modulation = v;
    emit modulatedValueChanged();
}

bool ClapPluginParam::isValueValid(const double v) const {
    return _info.min_value <= v && v <= _info.max_value;
}

void ClapPluginParam::printShortInfo(std::ostream &os) const {
    os << "id: " << _info.id << ", name: '" << _info.name << "', module: '" << _info.module << "'";
}

void ClapPluginParam::printInfo(std::ostream &os) const {
    printShortInfo(os);
    os << ", min: " << _info.min_value << ", max: " << _info.max_value;
}

bool ClapPluginParam::isInfoEqualTo(const clap_param_info &info) const {
    return info.cookie == _info.cookie &&
           info.default_value == _info.default_value &&
           info.max_value == _info.max_value &&
           info.min_value == _info.min_value &&
           info.flags == _info.flags &&
           info.id == _info.id &&
           !::std::strncmp(info.name, _info.name, sizeof (info.name)) &&
           !::std::strncmp(info.module, _info.module, sizeof (info.module));
}

bool ClapPluginParam::isInfoCriticallyDifferentTo(const clap_param_info &info) const {
    assert(_info.id == info.id);
    const uint32_t criticalFlags =
        CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_AUTOMATABLE_PER_NOTE_ID |
        CLAP_PARAM_IS_AUTOMATABLE_PER_KEY | CLAP_PARAM_IS_AUTOMATABLE_PER_CHANNEL |
        CLAP_PARAM_IS_AUTOMATABLE_PER_PORT | CLAP_PARAM_IS_MODULATABLE |
        CLAP_PARAM_IS_MODULATABLE_PER_NOTE_ID | CLAP_PARAM_IS_MODULATABLE_PER_KEY |
        CLAP_PARAM_IS_MODULATABLE_PER_CHANNEL | CLAP_PARAM_IS_MODULATABLE_PER_PORT |
        CLAP_PARAM_IS_READONLY | CLAP_PARAM_REQUIRES_PROCESS;
    return (_info.flags & criticalFlags) == (info.flags & criticalFlags) ||
           _info.min_value != _info.min_value || _info.max_value != _info.max_value;
}
