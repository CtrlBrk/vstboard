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
#ifndef CLAPPLUGINPARAM_H
#define CLAPPLUGINPARAM_H

#include <ostream>
#include <unordered_map>

#include <QObject>

#include <clap/clap.h>

namespace Connectables {
class ClapPlugin;
}
class ClapPluginParam : public QObject {
    Q_OBJECT;

public:
    ClapPluginParam(Connectables::ClapPlugin &pluginHost, const clap_param_info &info, double value);

    double value() const { return _value; }
    void setValue(double v);

    double modulation() const { return _modulation; }
    void setModulation(double v);

    double modulatedValue() const {
        return std::min(_info.max_value, std::max(_info.min_value, _value + _modulation));
    }

    bool isValueValid(const double v) const;

    void printShortInfo(std::ostream &os) const;
    void printInfo(std::ostream &os) const;

    void setInfo(const clap_param_info &info) noexcept { _info = info; }
    bool isInfoEqualTo(const clap_param_info &info) const;
    bool isInfoCriticallyDifferentTo(const clap_param_info &info) const;
    clap_param_info &info() noexcept { return _info; }
    const clap_param_info &info() const noexcept { return _info; }

    bool isBeingAdjusted() const noexcept { return _isBeingAdjusted; }
    void setIsAdjusting(bool isAdjusting) {
        if (isAdjusting && !_isBeingAdjusted)
            beginAdjust();
        else if (!isAdjusting && _isBeingAdjusted)
            endAdjust();
    }
    void beginAdjust() {
        Q_ASSERT(!_isBeingAdjusted);
        _isBeingAdjusted = true;
        emit isBeingAdjustedChanged();
    }
    void endAdjust() {
        Q_ASSERT(_isBeingAdjusted);
        _isBeingAdjusted = false;
        emit isBeingAdjustedChanged();
    }

    float FloatFromValue() const;
    double ValueFromFloat(float val) const;
    int pinNumber = 0;

signals:
    void isBeingAdjustedChanged();
    void infoChanged();
    void valueChanged();
    void modulatedValueChanged();

private:
    bool _isBeingAdjusted = false;
    clap_param_info _info;
    double _value = 0;
    double _modulation = 0;
    std::unordered_map<int64_t, std::string> _enumEntries;

};

#endif // CLAPPLUGINPARAM_H
