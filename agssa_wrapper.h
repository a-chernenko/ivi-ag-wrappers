/*

MIT License

Copyright (c) 2018 Alexander Chernenko (achernenko@mail.ru)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/******************************************************************************
[Use example]
-------------------------------------------------------------------------------
  ::AgSsa::CAgSsa sigSAn{};

  sigSAn.Application.PN.Aquisition.ConfigureSweepModeContinuous(false);
  sigSAn.Application.PN.Measurement.Spurious.ConfigurePower();
  sigSAn.Application.PN.Display.ConfigureMaximize();
  sigSAn.Application.PN.Frequency.ConfigureFrequencyBand(1000);
  sigSAn.Display.ConfigureActiveWindow(::Display::ActiveWindowType::PN1);
  sigSAn.Display.ConfigureMaximize();
  sigSAn.Trigger.Mode(::Display::ActiveWindowType::PN1);
  sigSAn.Trigger.ConfigureSOPC();
  sigSAn.Application.PN.Measurements.Initiate();
  sigSAn.System.WaitForOperationComplete(1min);

  CSpursData spursData{};
  sigSAn.Application.PN.Measurements.QuerySpuriousList(spursData);
******************************************************************************/

#ifndef AGSSA_WRAPPER_H
#define AGSSA_WRAPPER_H

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "AgSsa.h"
#include "visa.h"

#include "ivi_inner_session.h"

namespace AgSsa {

enum class AgSsaModel : ViUInt32 { Common = 0, E5052B };

namespace Utility {

class CAgSsaUtility : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto Reset() const noexcept { return AgSsa_reset(m_Session); }
  auto ClearError() const noexcept { return AgSsa_ClearError(m_Session); }
  template <ViInt32 size>
  auto GerError(ViStatus &code, std::array<ViChar, size> &description) const
      noexcept {
    return AgSsa_GetError(m_Session, &code, size, description.data());
  }
};

}  // namespace Utility

namespace System {

class CAgSsaSystem : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto WaitForOperationComplete(const std::chrono::milliseconds &timeout) const
      noexcept {
    return AgSsa_SystemWaitForOperationComplete(m_Session,
                                                ViInt32(timeout.count()));
  }
};

}  // namespace System

namespace Display {

enum class ActiveWindowType : ViInt32 {
  PN1 = AGSSA_VAL_DISPLAY_ACTIVE_WINDOW_PN1
};

class CAgSsaDisplay : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ConfigureMaximize(bool value = true) const noexcept {
    return AgSsa_SetAttributeViBoolean(m_Session, nullptr,
                                       AGSSA_ATTR_DISPLAY_MAXIMIZE, value);
  }
  auto ConfigureActiveWindow(ActiveWindowType value) const noexcept {
    return AgSsa_SetAttributeViInt32(
        m_Session, nullptr, AGSSA_ATTR_DISPLAY_ACTIVE_WINDOW,
        std::underlying_type<ActiveWindowType>::type(value));
  }
};

}  // namespace Display

namespace Trigger {

class CAgSsaTrigger : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto Mode(Display::ActiveWindowType value) const noexcept {
    return AgSsa_SetAttributeViInt32(
        m_Session, nullptr, AGSSA_ATTR_TRIGGER_MODE,
        std::underlying_type<Display::ActiveWindowType>::type(value));
  }
  auto ConfigureSOPC(bool enabled = true) const noexcept {
    return AgSsa_SetAttributeViBoolean(
        m_Session, nullptr, AGSSA_ATTR_TRIGGER_SOPC_ENABLED, enabled);
  }
};

}  // namespace Trigger

namespace Application {

namespace PN {

namespace Measurements {

struct CCarrierData {
  ViReal64 Frequency{};
  ViReal64 Power{};
};

struct CSpurData {
  ViReal64 Frequency{};
  ViReal64 Amplitude{};
  ViReal64 Unknown{};
};

using CSpursData = std::vector<CSpurData>;

class CAgSsaApplicationPNMeasurements : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto Initiate() const noexcept {
    return AgSsa_ApplicationPhaseNoiseMeasurementsInitiate(m_Session);
  }
  auto QueryCarrierData(CCarrierData &data) const noexcept {
    CCarrierData retData{};
    ViInt32 retSize{};
    auto status = AgSsa_ApplicationPhaseNoiseMeasurementsGet_CarrierData(
        m_Session, sizeof(CCarrierData), reinterpret_cast<ViReal64 *>(&retData),
        &retSize);
    if ((status == VI_SUCCESS) &&
        (retSize == sizeof(CCarrierData) / sizeof(ViReal64))) {
      data = retData;
    }
    return status;
  }
  auto QuerySpuriousList(CSpursData &spursData) const noexcept {
    auto status =
        AgSsa_SystemWriteString(m_Session, ":CALC:PN1:TRAC1:SPUR:SLIS?");
    if (status != VI_SUCCESS) return status;
    ViInt64 retSize{};
    std::array<ViChar, 8192> retBuf{};
    std::string retString;
    do {
      status = AgSsa_viRead(m_Session, retBuf.size(), retBuf.data(), &retSize);
      retString += std::string(retBuf.data(), std::size_t(retSize));
    } while (status == VI_SUCCESS_MAX_CNT);
    if (status != VI_SUCCESS) return status;
    auto splitSeparatedString = [](const std::string &unsplitted,
                                   const std::string &separator) {
      std::vector<std::string> splitted{};
      std::string::size_type start{};
      std::string::size_type index = unsplitted.find(separator);
      while (index != std::string::npos) {
        splitted.push_back(unsplitted.substr(start, index));
        index += separator.length();
        start += index;
        index = unsplitted.substr(start).find(separator);
      };
      splitted.push_back(unsplitted.substr(start));
      return splitted;
    };
    auto splittedString = splitSeparatedString(retString, ",");
    [&spursData, &splittedString] {
      CSpurData spurData;
      auto spurParamsNum{sizeof(CSpurData) / sizeof(ViReal64)};
      for (std::size_t idx{}; idx < splittedString.size();
           idx += spurParamsNum) {
        std::string::size_type sz;
        spurData.Frequency = std::stod(splittedString[idx], &sz);
        spurData.Amplitude = std::stod(splittedString[idx + 1], &sz);
        spurData.Unknown = std::stod(splittedString[idx + 2], &sz);
        spursData.push_back(spurData);
      }
    }();
    return status;
  }
  auto Abort() const noexcept {
    return AgSsa_ApplicationPhaseNoiseMeasurementsAbort(m_Session);
  }
};

}  // namespace Measurements

namespace Measurement {

namespace Spurious {

class CAgSsaApplicationPNMeasurementSpurious : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ConfigurePower(bool value = true) const noexcept {
    return AgSsa_SetAttributeViBoolean(
        m_Session, "Measurement1",
        AGSSA_ATTR_APPLICATION_PHASENOISE_MEASUREMENT_SPURIOUS_POWER, value);
  }
};

}  // namespace Spurious

class CAgSsaApplicationPNMeasurement : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  Spurious::CAgSsaApplicationPNMeasurementSpurious const Spurious{m_Session};
};

}  // namespace Measurement

namespace Aquisition {

class CAgSsaApplicationPNAquisition : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ConfigureCorrelation(int value) const noexcept {
    return AgSsa_SetAttributeViInt32(
        m_Session, nullptr,
        AGSSA_ATTR_APPLICATION_PHASENOISE_ACQUISITION_CORRELATION, value);
  }
  auto QueryCorrelation(int &value) const noexcept {
    ViInt32 correlation{};
    auto status = AgSsa_GetAttributeViInt32(
        m_Session, nullptr,
        AGSSA_ATTR_APPLICATION_PHASENOISE_ACQUISITION_CORRELATION,
        &correlation);
    if (status == VI_SUCCESS) {
      value = static_cast<int>(correlation);
    }
    return status;
  }
  auto ConfigureSweepModeContinuous(bool enabled = true) const noexcept {
    return AgSsa_SetAttributeViBoolean(
        m_Session, nullptr,
        AGSSA_ATTR_APPLICATION_PHASENOISE_ACQUISITION_SWEEP_MODE_CONTINUOUS,
        enabled);
  }
};

}  // namespace Aquisition

namespace Display {

class CAgSsaApplicationPNDisplay : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ConfigureMaximize(bool maximized = true) const noexcept {
    return AgSsa_SetAttributeViBoolean(
        m_Session, nullptr, AGSSA_ATTR_APPLICATION_PHASENOISE_DISPLAY_MAXIMIZE,
        maximized);
  }
};

}  // namespace Display

namespace Frequency {

enum class FrequencyStartOffset : ViInt32 {
  _1Hz = 1,
  _10Hz = 10,
  _100Hz = 100,
  _1kHz = 1000
};

enum class FrequencyStopOffset : ViInt32 {
  _100kHz = 100'000,
  _1MHz = 1'000'000,
  _5MHz = 5'000'000,
  _10MHz = 10'000'000,
  _20MHz = 20'000'000,
  _40MHz = 40'000'000,
  _100MHz = 100'000'000,
};

enum class FrequencyBand : ViInt32 {
  BAND1 = AGSSA_VAL_AGILENT_SSA_FREQUENCY_BAND1,
  BAND2 = AGSSA_VAL_FREQUENCY_BAND2,
  BAND3 = AGSSA_VAL_FREQUENCY_BAND3,
  BAND4 = AGSSA_VAL_FREQUENCY_BAND4,
  BAND5 = AGSSA_VAL_FREQUENCY_BAND5,
  BAND6 = AGSSA_VAL_FREQUENCY_BAND6,
  BAND_LOW = AGSSA_VAL_FREQUENCY_BAND_LOW,
  BAND_HIGH = AGSSA_VAL_FREQUENCY_BAND_HIGH
};

class CAgSsaApplicationPNFrequency : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ConfigureFrequencyBand(FrequencyBand value) const noexcept {
    return AgSsa_SetAttributeViInt32(
        m_Session, nullptr, AGSSA_ATTR_APPLICATION_PHASENOISE_FREQUENCY_BAND,
        std::underlying_type<FrequencyBand>::type(value));
  }
  auto QueryFrequencyBand(FrequencyBand &value) const noexcept {
    ViInt32 rawBand{};
    auto status = AgSsa_GetAttributeViInt32(
        m_Session, nullptr, AGSSA_ATTR_APPLICATION_PHASENOISE_FREQUENCY_BAND,
        &rawBand);
    if (status == VI_SUCCESS) {
      FrequencyBand band{std::underlying_type<FrequencyBand>::type(rawBand)};
      status = VI_ERROR_INV_RESPONSE;
      switch (band) {
        case FrequencyBand::BAND1:
        case FrequencyBand::BAND2:
        case FrequencyBand::BAND3:
        case FrequencyBand::BAND4:
        case FrequencyBand::BAND5:
        case FrequencyBand::BAND6:
        case FrequencyBand::BAND_LOW:
        case FrequencyBand::BAND_HIGH:
          value = band;
          status = VI_SUCCESS;
          break;
      }
    }
    return status;
  }
  auto ConfigureStartOffset(FrequencyStartOffset value) const noexcept {
    auto rawFrequency = static_cast<ViReal64>(value);
    return AgSsa_SetAttributeViReal64(
        m_Session, nullptr,
        AGSSA_ATTR_APPLICATION_PHASENOISE_FREQUENCY_START_OFFSET, rawFrequency);
  }
  auto QueryStartOffset(FrequencyStartOffset &value) const noexcept {
    ViReal64 rawFrequency{};
    auto status = AgSsa_GetAttributeViReal64(
        m_Session, nullptr,
        AGSSA_ATTR_APPLICATION_PHASENOISE_FREQUENCY_START_OFFSET,
        &rawFrequency);
    if (status == VI_SUCCESS) {
      FrequencyStartOffset frequency{
          std::underlying_type<FrequencyStartOffset>::type(rawFrequency)};
      status = VI_ERROR_INV_RESPONSE;
      switch (frequency) {
        case FrequencyStartOffset::_1Hz:
        case FrequencyStartOffset::_10Hz:
        case FrequencyStartOffset::_100Hz:
        case FrequencyStartOffset::_1kHz:
          value = frequency;
          status = VI_SUCCESS;
          break;
      }
    }
    return status;
  }
  auto ConfigureStopOffset(FrequencyStopOffset value) const noexcept {
    auto rawFrequency = static_cast<ViReal64>(value);
    return AgSsa_SetAttributeViReal64(
        m_Session, nullptr,
        AGSSA_ATTR_APPLICATION_PHASENOISE_FREQUENCY_STOP_OFFSET, rawFrequency);
  }
  auto QueryStopOffset(FrequencyStopOffset &value) const noexcept {
    ViReal64 rawFrequency{};
    auto status = AgSsa_GetAttributeViReal64(
        m_Session, nullptr,
        AGSSA_ATTR_APPLICATION_PHASENOISE_FREQUENCY_STOP_OFFSET, &rawFrequency);
    if (status == VI_SUCCESS) {
      FrequencyStopOffset frequency{
          std::underlying_type<FrequencyStopOffset>::type(rawFrequency)};
      status = VI_ERROR_INV_RESPONSE;
      switch (frequency) {
        case FrequencyStopOffset::_100kHz:
        case FrequencyStopOffset::_1MHz:
        case FrequencyStopOffset::_5MHz:
        case FrequencyStopOffset::_10MHz:
        case FrequencyStopOffset::_20MHz:
        case FrequencyStopOffset::_40MHz:
        case FrequencyStopOffset::_100MHz:
          value = frequency;
          status = VI_SUCCESS;
          break;
      }
    }
    return status;
  }
};

}  // namespace Frequency

class CAgSsaApplicationPN : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto AutoSettings() const noexcept {
    std::string_view PN_ASET{"SENS:PS1:ASET"};
    return AgSsa_SystemWrite(m_Session, PN_ASET.data());
  }
  Frequency::CAgSsaApplicationPNFrequency const Frequency{m_Session};
  Aquisition::CAgSsaApplicationPNAquisition const Aquisition{m_Session};
  Display::CAgSsaApplicationPNDisplay const Display{m_Session};
  Measurement::CAgSsaApplicationPNMeasurement const Measurement{m_Session};
  Measurements::CAgSsaApplicationPNMeasurements const Measurements{m_Session};
};

}  // namespace PN

class CAgSsaApplication : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  PN::CAgSsaApplicationPN const PN{m_Session};
};

}  // namespace Application

struct CAgSsaOptions {
  CAgSsaOptions() = default;
  template <class COptions>
  CAgSsaOptions(const COptions &opt)
      : Model(static_cast<AgSsaModel>(opt.Model)),
        Simulate(opt.Simulate),
        Reset(opt.Reset),
        idQuery(opt.idQuery) {}
  AgSsaModel Model{AgSsaModel::Common};
  bool Simulate{};
  bool Reset{};
  bool idQuery{};
};

class CAgSsa {
  ViSession m_Session{};
  CAgSsaOptions m_Options{};
  std::string MakeOptionsString(const CAgSsaOptions &options) {
    using namespace std::string_literals;
    auto opts = "Simulate="s;
    m_Options = options;
    if (options.Simulate) {
      opts += "true"s;
    } else {
      opts += "false"s;
    }
    if (options.Model != AgSsaModel::Common) {
      opts += ", DriverSetup=Model="s;
      switch (options.Model) {
        case AgSsaModel::E5052B:
          opts += "E5052B"s;
          break;
        default:
          opts.clear();
      }
    }
    return opts;
  }

 public:
  auto Connect(const std::string &resource,
               const CAgSsaOptions &options) noexcept {
    auto optionsString = MakeOptionsString(options);
    return AgSsa_InitWithOptions(ViRsrc(resource.data()), options.idQuery,
                                 options.Reset, optionsString.data(),
                                 &m_Session);
  }
  void Close() noexcept {
    AgSsa_close(m_Session);
    m_Session = 0;
  }
  bool IsOpen() const noexcept { return (m_Session != 0); }
  bool IsSimulate() const noexcept { return m_Options.Simulate; }
  ViSession GetSession() const noexcept { return m_Session; }
  Application::CAgSsaApplication const Application{m_Session};
  Display::CAgSsaDisplay const Display{m_Session};
  Trigger::CAgSsaTrigger const Trigger{m_Session};
  System::CAgSsaSystem const System{m_Session};
  Utility::CAgSsaUtility const Utility{m_Session};
};

}  // namespace AgSsa

#endif  // AGSSA_WRAPPER_H
