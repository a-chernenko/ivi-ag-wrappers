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
  ::AgXSAn::CAgXSAn specAn{};

  specAn.Input.Rf.Corrections.ConfigureFloorExtentionEnabled();
  specAn.SA.SweptSAs.Configure();
  specAn.SA.SpuriousEmissions.Configure();
  specAn.SA.SpuriousEmissions.FastMeasurementEnabled();
  specAn.Acquisition.ContiniousSweepModeEnabled();
  specAn.SA.SweptSAs.Initiate();
  specAn.Frequency.Tune();
  specAn.System.WaitForOperationComplete(5s);

  CAgXSAnSweepPointsAutoEnabledTable spurSweepPointsAutoEnabledTable{};
  spurSweepPointsAutoEnabledTable.fill(true);
  specAn.SA.SpuriousEmissions.RangeTable.ConfigureSweepPointsAutoEnabled(
          spurSweepPointsAutoEnabledTable);
******************************************************************************/

#ifndef AGXSAN_WRAPPER_H
#define AGXSAN_WRAPPER_H

#include <array>
#include <chrono>
#include <functional>
#include <string>

#include "AgXSAn.h"
#include "visa.h"

#include "ivi_inner_session.h"

namespace AgXSAn {

enum class AgXSAnModel : ViUInt32 { Common = 0, N9030A };

namespace SA {

namespace SpuriousEmissions {

namespace Types {

template <typename ElementType, ViInt32 size>
struct CTable : std::array<ElementType, size> {
  using Type = std::array<ElementType, size>;
  using std::array<ElementType, size>::array;
  CTable(std::array<ElementType, size> table)
      : std::array<ElementType, size>(table) {}
};

template <ViInt32 size>
struct CAttenuationTable : CTable<ViReal64, size> {
  using CTable<ViReal64, size>::CTable;
};

template <ViInt32 size>
struct CResolutionTable : CTable<ViReal64, size> {
  using CTable<ViReal64, size>::CTable;
};

template <ViInt32 size>
struct CSweepPointsAutoEnabledTable : CTable<ViBoolean, size> {
  using CTable<ViBoolean, size>::CTable;
};

template <ViInt32 size>
struct CPeakThresholdTable : CTable<ViReal64, size> {
  using CTable<ViReal64, size>::CTable;
};

template <ViInt32 size>
struct CSweepTimeTable : CTable<ViReal64, size> {
  using CTable<ViReal64, size>::CTable;
};

template <ViInt32 size>
struct CEnabledTable : CTable<ViBoolean, size> {
  using CTable<ViBoolean, size>::CTable;
};

template <ViInt32 size>
struct CFrequencyTable : CTable<ViReal64, size> {
  using CTable<ViReal64, size>::CTable;
};

template <ViInt32 size>
struct CAbsoluteAmplitudeLimitTable : CTable<ViReal64, size> {
  using CTable<ViReal64, size>::CTable;
};

template <ViInt32 size>
struct CAbsoluteAmplitudeLimitAutoEnabledTable : CTable<ViBoolean, size> {
  using CTable<ViBoolean, size>::CTable;
};

struct CSpurData {
  ViReal64 Number;
  ViReal64 Range;
  ViReal64 Frequency;
  ViReal64 Amplitude;
  ViReal64 Limit;
  ViReal64 Unknown;
};

using CSpursData = std::vector<CSpurData>;

struct AgXSAnConstatns {
  inline static constexpr ViInt32 RangeTableMax{20};
};

using CAgXSAnAttenuationTable =
    CAttenuationTable<AgXSAnConstatns::RangeTableMax>;
using CAgXSAnEnabledTable = CEnabledTable<AgXSAnConstatns::RangeTableMax>;
using CAgXSAnFrequencyTable = CFrequencyTable<AgXSAnConstatns::RangeTableMax>;
using CAgXSAnAbsoluteAmplitudeLimitTable =
    CAbsoluteAmplitudeLimitTable<AgXSAnConstatns::RangeTableMax>;
using CAgXSAnSweepPointsAutoEnabledTable =
    CSweepPointsAutoEnabledTable<AgXSAnConstatns::RangeTableMax>;
using CAgXSAnPeakThresholdTable =
    CPeakThresholdTable<AgXSAnConstatns::RangeTableMax>;
using CAgXSAnSweepTimeTable = CSweepTimeTable<AgXSAnConstatns::RangeTableMax>;
using CAgXSAnResolutionTable = CResolutionTable<AgXSAnConstatns::RangeTableMax>;
using CAgXSAnAbsoluteAmplitudeLimitAutoEnabledTable =
    CAbsoluteAmplitudeLimitAutoEnabledTable<AgXSAnConstatns::RangeTableMax>;

struct AgXSAnPresets {
  struct Display {
    struct Window {
      inline static constexpr ViReal64 Reference = 0;
      inline static constexpr ViReal64 Scale = 10;
    };
  };
  struct Bandwidth {
    inline static constexpr CAgXSAnResolutionTable::Type ResolutionTable = {
        1.2E6, 0.51E6, 0.1E6, 0.1E6, 4E6, 3E6, 3E6, 3E6, 3E6, 3E6,
        3.0E6, 3.00E6, 3.0E6, 3.0E6, 3E6, 3E6, 3E6, 3E6, 3E6, 3E6};
  };

  struct RangeTable {
    inline static constexpr CAgXSAnAttenuationTable::Type AttenuationTable = {
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
    inline static constexpr CAgXSAnPeakThresholdTable::Type PeakThresholdTable =
        {-9E1, -9E1, -9E1, -9E1, -9E1, -9E1, -9E1, -9E1, -9E1, -9E1,
         -9E1, -9E1, -9E1, -9E1, -9E1, -9E1, -9E1, -9E1, -9E1, -9E1};
    inline static constexpr CAgXSAnEnabledTable::Type EnabledTable = {
        VI_TRUE,  VI_TRUE,  VI_TRUE,  VI_TRUE,  VI_TRUE,  VI_FALSE, VI_FALSE,
        VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE,
        VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE};
    inline static constexpr CAgXSAnSweepPointsAutoEnabledTable::Type
        SweepPointsAutoEnabledTable = {VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE,
                                       VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE,
                                       VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE,
                                       VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE,
                                       VI_FALSE, VI_FALSE, VI_FALSE, VI_FALSE};
    struct Start {
      inline static constexpr CAgXSAnFrequencyTable::Type FrequencyTable = {
          +1.92E9, +1.8935E9, +2.1E9, +2.175E9, +8.00E8,   +1.5E9, +1.5E9,
          +1.5E9,  +1.5E9,    +1.5E9, +1.50E9,  +1.5000E9, +1.5E9, +1.500E9,
          +1.50E9, +1.5E9,    +1.5E9, +1.5E9,   +1.5E9,    +1.5E9};
      inline static constexpr CAgXSAnAbsoluteAmplitudeLimitTable::Type
          AbsoluteAmplitudeLimitTable = {
              -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1,
              -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1};
    };
    struct Stop {
      inline static constexpr CAgXSAnFrequencyTable::Type FrequencyTable = {
          +1.98E9, +1.9196E9, +2.1015E9, +2.18E9, +1.0E9,    +2.5E9,    +2.5E9,
          +2.5E9,  +2.5E9,    +2.5E9,    +2.50E9, +2.5000E9, +2.5000E9, +2.50E9,
          +2.5E9,  +2.5E9,    +2.5E9,    +2.5E9,  +2.5E9,    +2.5E9};
      inline static constexpr CAgXSAnAbsoluteAmplitudeLimitTable::Type
          AbsoluteAmplitudeLimitTable = {
              -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1,
              -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1, -5E1};
      inline static constexpr CAgXSAnAbsoluteAmplitudeLimitAutoEnabledTable::
          Type AbsoluteAmplitudeLimitAutoEnabledTable = {
              VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE,
              VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE,
              VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE, VI_TRUE};
    };
  };
};

template <ViInt32 size>
void staticAssertRangeTable() {
  static_assert(size <= AgXSAnConstatns::RangeTableMax,
                "Table size is excessed!");
  static_assert(size > 0, "Table size must be greater then zero!");
}

template <size_t size>
void staticAssertArgsSize() {
  static_assert(size <= AgXSAnConstatns::RangeTableMax,
                "Args size is excessed!");
  static_assert(size > 0, "Args size must be greater then zero!");
}

}  // namespace Types

namespace Trace {

class CAgXSAnSASpuriousEmissionsTrace : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;
  using GetSpuriousResultsFunctorType =
      std::function<ViStatus(ViInt32, ViReal64 *, ViInt32 *)>;
  auto GetSpuriousResults(Types::CSpursData &spursData,
                          const GetSpuriousResultsFunctorType &functor) const
      noexcept {
    using namespace Types;
    const ViInt32 querySpursNum{256};
    const ViInt32 spurParamsNum{sizeof(CSpurData) / sizeof(ViReal64)};
    const ViInt32 queryBufSize{querySpursNum * spurParamsNum};
    ViReal64 buf[queryBufSize]{};
    ViInt32 retBufSize{};
    auto status = functor(queryBufSize, buf, &retBufSize);
    if (status == VI_SUCCESS) {
      const ViInt32 retSpursNum{retBufSize / spurParamsNum};
      spursData.reserve(std::size_t(retSpursNum));
      for (ViInt32 idx{}; idx < retSpursNum; ++idx) {
        CSpurData spur = *(reinterpret_cast<CSpurData *>(&buf[1]) + idx);
        spursData.push_back(spur);
      }
    }
    return status;
  }

 public:
  auto ReadSpuriousResults(Types::CSpursData &spursData,
                           const std::chrono::milliseconds &timeout) const
      noexcept {
    using namespace std::placeholders;
    const GetSpuriousResultsFunctorType traceRead = std::bind(
        AgXSAn_SASpuriousEmissionsTraceRead, m_Session,
        std::cref("Spurious_Results"), ViInt32(timeout.count()), _1, _2, _3);
    return GetSpuriousResults(spursData, traceRead);
  }
  auto FetchSpuriousResults(Types::CSpursData &spursData) const noexcept {
    using namespace std::placeholders;
    const GetSpuriousResultsFunctorType traceFetch =
        std::bind(AgXSAn_SASpuriousEmissionsTraceFetch, m_Session,
                  std::cref("Spurious_Results"), _1, _2, _3);
    return GetSpuriousResults(spursData, traceFetch);
  }
};

}  // namespace Trace

namespace Traces {

class CAgXSAnSASpuriousEmissionsTraces : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto Abort() const noexcept {
    return AgXSAn_SASpuriousEmissionsTracesAbort(m_Session);
  }
  auto Initiate() const noexcept {
    return AgXSAn_SASpuriousEmissionsTracesInitiate(m_Session);
  }
};

}  // namespace Traces

namespace RangeTable {

namespace Start {

class CAgXSAnSASpuriousEmissionsRangeTableStart : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  template <ViInt32 size>
  auto ConfigureFrequency(Types::CFrequencyTable<size> &table) const noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableStartConfigureFrequency(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViReal64>...>,
                   ViStatus>
  ConfigureFrequency(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CFrequencyTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableStartConfigureFrequency(
        m_Session, tmp.size(), tmp.data());
  }
  template <ViInt32 size>
  auto ConfigureAbsoluteAmplitudeLimit(
      Types::CAbsoluteAmplitudeLimitTable<size> &table) const noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableStartConfigureAbsoluteAmplitudeLimit(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViReal64>...>,
                   ViStatus>
  ConfigureAbsoluteAmplitudeLimit(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CAbsoluteAmplitudeLimitTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableStartConfigureAbsoluteAmplitudeLimit(
        m_Session, tmp.size(), tmp.data());
  }
};

}  // namespace Start

namespace Stop {

class CAgXSAnSASpuriousEmissionsRangeTableStop : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  template <ViInt32 size>
  auto ConfigureFrequency(Types::CFrequencyTable<size> &table) const noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableStopConfigureFrequency(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViReal64>...>,
                   ViStatus>
  ConfigureFrequency(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CFrequencyTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableStopConfigureFrequency(
        m_Session, tmp.size(), tmp.data());
  }
  template <ViInt32 size>
  auto ConfigureAbsoluteAmplitudeLimit(
      Types::CAbsoluteAmplitudeLimitTable<size> &table) const noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableStopConfigureAbsoluteAmplitudeLimit(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViReal64>...>,
                   ViStatus>
  ConfigureAbsoluteAmplitudeLimit(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CAbsoluteAmplitudeLimitTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableStopConfigureAbsoluteAmplitudeLimit(
        m_Session, tmp.size(), tmp.data());
  }
  template <ViInt32 size>
  auto ConfigureAbsoluteAmplitudeLimitAutoEnabled(
      Types::CAbsoluteAmplitudeLimitAutoEnabledTable<size> &table) const
      noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableStopConfigureAbsoluteAmplitudeLimitAutoEnabled(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViBoolean>...>,
                   ViStatus>
  ConfigureAbsoluteAmplitudeLimitAutoEnabled(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CAbsoluteAmplitudeLimitAutoEnabledTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableStopConfigureAbsoluteAmplitudeLimitAutoEnabled(
        m_Session, tmp.size(), tmp.data());
  }
};

}  // namespace Stop

namespace Bandwidth {

class CAgXSAnSASpuriousEmissionsRangeTableBandwidth
    : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  template <ViInt32 size>
  auto ConfigureResolution(Types::CResolutionTable<size> &table) const
      noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableBandwidthConfigureResolution(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViReal64>...>,
                   ViStatus>
  ConfigureResolution(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CResolutionTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableBandwidthConfigureResolution(
        m_Session, tmp.size(), tmp.data());
  }
};

}  // namespace Bandwidth

class CAgXSAnSASpuriousEmissionsRangeTable : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  template <ViInt32 size>
  auto ConfigureEnabled(Types::CEnabledTable<size> &table) const noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableConfigureEnabled(m_Session, size,
                                                                table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViBoolean>...>,
                   ViStatus>
  ConfigureEnabled(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CEnabledTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableConfigureEnabled(
        m_Session, tmp.size(), tmp.data());
  }
  template <ViInt32 size>
  auto ConfigureAttenuation(Types::CAttenuationTable<size> &table) const
      noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableConfigureAttenuation(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViReal64>...>,
                   ViStatus>
  ConfigureAttenuation(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CAttenuationTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableConfigureAttenuation(
        m_Session, tmp.size(), tmp.data());
  }
  template <ViInt32 size>
  auto ConfigureSweepPointsAutoEnabled(
      Types::CSweepPointsAutoEnabledTable<size> &table) const noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableConfigureSweepPointsAutoEnabled(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViBoolean>...>,
                   ViStatus>
  ConfigureSweepPointsAutoEnabled(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CSweepPointsAutoEnabledTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableConfigureSweepPointsAutoEnabled(
        m_Session, tmp.size(), tmp.data());
  }
  template <ViInt32 size>
  auto QuerySweepTime(Types::CSweepTimeTable<size> &table) const noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    ViInt32 retBufSize{};
    return AgXSAn_SASpuriousEmissionsRangeTableQuerySweepTime(
        m_Session, size, table.data(), &retBufSize);
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViReal64>...>,
                   ViStatus>
  QuerySweepTime(Args &... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CSweepTimeTable<sizeof...(args)> tmp{{args...}};
    ViInt32 retBufSize{};
    return AgXSAn_SASpuriousEmissionsRangeTableQuerySweepTime(
        m_Session, tmp.size(), tmp.data(), &retBufSize);
  }
  template <ViInt32 size>
  auto ConfigurePeakThreshold(Types::CPeakThresholdTable<size> &table) const
      noexcept {
    using namespace Types;
    staticAssertRangeTable<size>();
    return AgXSAn_SASpuriousEmissionsRangeTableConfigurePeakThreshold(
        m_Session, size, table.data());
  }
  template <typename... Args>
  std::enable_if_t<std::conjunction_v<std::is_same<Args, ViReal64>...>,
                   ViStatus>
  ConfigurePeakThreshold(Args... args) const noexcept {
    using namespace Types;
    staticAssertArgsSize<sizeof...(args)>();
    CPeakThresholdTable<sizeof...(args)> tmp{{args...}};
    return AgXSAn_SASpuriousEmissionsRangeTableConfigurePeakThreshold(
        m_Session, tmp.size(), tmp.data());
  }

  Bandwidth::CAgXSAnSASpuriousEmissionsRangeTableBandwidth const Badwidth{
      m_Session};
  Start::CAgXSAnSASpuriousEmissionsRangeTableStart const Start{m_Session};
  Stop::CAgXSAnSASpuriousEmissionsRangeTableStop const Stop{m_Session};
};

}  // namespace RangeTable

namespace Display {

namespace Window {

class CAgXSAnSASpuriousEmissionsDisplayWindow : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ConfigureReference(ViReal64 value) const noexcept {
    return AgXSAn_SetAttributeViReal64(
        m_Session, nullptr,
        AGXSAN_ATTR_SA_SPURIOUSEMISSIONS_DISPLAY_WINDOWY_REFERENCE, value);
  }
  auto ConfigureScale(ViReal64 value) const noexcept {
    return AgXSAn_SetAttributeViReal64(
        m_Session, nullptr,
        AGXSAN_ATTR_SA_SPURIOUSEMISSIONS_DISPLAY_WINDOWY_SCALE, value);
  }
};

}  // namespace Window

class CAgXSAnSASpuriousEmissionsDisplay : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  Window::CAgXSAnSASpuriousEmissionsDisplayWindow const Window{m_Session};
};

}  // namespace Display

class CAgXSAnSASpuriousEmissions : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto Configure() const noexcept {
    return AgXSAn_SASpuriousEmissionsConfigure(m_Session);
  }
  auto FastMeasurementEnabled(bool enabled = true) const noexcept {
    return AgXSAn_SetAttributeViBoolean(
        m_Session, nullptr,
        AGXSAN_ATTR_SA_SPURIOUSEMISSIONS_FAST_MEASUREMENT_ENABLED, enabled);
  }
  Traces::CAgXSAnSASpuriousEmissionsTraces const Traces{m_Session};
  Trace::CAgXSAnSASpuriousEmissionsTrace const Trace{m_Session};
  RangeTable::CAgXSAnSASpuriousEmissionsRangeTable const RangeTable{m_Session};
  Display::CAgXSAnSASpuriousEmissionsDisplay const Display{m_Session};
};

}  // namespace SpuriousEmissions

namespace SweptSAs {

class CAgXSAnSASweptSAs : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto Configure() const noexcept {
    return AgXSAn_SASweptSAsConfigure(m_Session);
  }
  auto Initiate() const noexcept {
    return AgXSAn_SASweptSAsInitiate(m_Session);
  }
};

}  // namespace SweptSAs

namespace Markers {

class CAgXSAnSAMarkers : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto SearchHighest() const noexcept {
    return AgXSAn_SAMarkerSearch(m_Session, AGXSAN_VAL_MARKER_SEARCH_HIGHEST);
  }
  auto Query(double &position, double &amplitude) const noexcept {
    return AgXSAn_SAMarkerQuery(m_Session, &position, &amplitude);
  }
};

}  // namespace Markers

class CAgXSAnSA : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  SpuriousEmissions::CAgXSAnSASpuriousEmissions const SpuriousEmissions{
      m_Session};
  SweptSAs::CAgXSAnSASweptSAs const SweptSAs{m_Session};
  Markers::CAgXSAnSAMarkers const Markers{m_Session};
};

}  // namespace SA

namespace System {

class CAgXSAnSystem : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ClearIO() const noexcept { return AgXSAn_SystemClearIO(m_Session); }
  auto WaitForOperationComplete(const std::chrono::milliseconds &timeout) const
      noexcept {
    return AgXSAn_SystemWaitForOperationComplete(m_Session,
                                                 ViInt32(timeout.count()));
  }
};

}  // namespace System

namespace Utility {

class CAgXSAnUtility : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto Reset() const noexcept { return AgXSAn_reset(m_Session); }
  auto ClearError() const noexcept { return AgXSAn_ClearError(m_Session); }
  template <ViInt32 size>
  auto GerError(ViStatus &code, std::array<ViChar, size> &description) const
      noexcept {
    return AgXSAn_GetError(m_Session, &code, size, description.data());
  }
};

}  // namespace Utility

namespace Input {

namespace Rf {

namespace Corrections {

class CAgXSAnInputRfCorrections : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ConfigureFloorExtentionEnabled(bool enabled = true) const noexcept {
    return AgXSAn_SetAttributeViBoolean(
        m_Session, nullptr,
        AGXSAN_ATTR_INPUT_RF_CORRECTIONS_NOISE_FLOOR_EXTENSTION_ENABLED,
        enabled);
  }
};

}  // namespace Corrections

class CAgXSAnInputRf : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  Corrections::CAgXSAnInputRfCorrections const Corrections{m_Session};
};

}  // namespace Rf

class CAgXSAnInput : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  Rf::CAgXSAnInputRf const Rf{m_Session};
};

}  // namespace Input

namespace Frequency {

class CAgXSAnFrequency : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto Tune() const noexcept { return AgXSAn_FrequencyTune(m_Session); }
};

}  // namespace Frequency

namespace Display {

class CAgXSAnDisplay : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto FullScreenEnabled(bool enabled = true) const noexcept {
    return AgXSAn_SetAttributeViBoolean(
        m_Session, nullptr, AGXSAN_ATTR_DISPLAY_FULL_SCREEN_ENABLED, enabled);
  }
};

}  // namespace Display

namespace BasicOperation {

class CAgXSAnBasicOperation : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto GetAttenuation(ViReal64 &value) const noexcept {
    return AgXSAn_GetAttributeViReal64(m_Session, nullptr,
                                       AGXSAN_ATTR_ATTENUATION, &value);
  }
};

}  // namespace BasicOperation

namespace Acquisition {

class CAgXSAnAcquisition : CIviInnerSessionReference {
  using CIviInnerSessionReference::CIviInnerSessionReference;

 public:
  auto ContiniousSweepModeEnabled(bool enabled = true) const noexcept {
    return AgXSAn_SetAttributeViBoolean(
        m_Session, nullptr,
        AGXSAN_ATTR_ACQUISITION_CONTINUOUS_SWEEP_MODE_ENABLED, enabled);
  }
};

}  // namespace Acquisition

struct CAgXSAnOptions {
  CAgXSAnOptions() = default;
  template <class COptions>
  CAgXSAnOptions(const COptions &opt)
      : Model(static_cast<AgXSAnModel>(opt.Model)),
        Simulate(opt.Simulate),
        Reset(opt.Reset),
        idQuery(opt.idQuery) {}
  AgXSAnModel Model{AgXSAnModel::Common};
  bool Simulate{};
  bool Reset{};
  bool idQuery{};
};

class CAgXSAn {
  ViSession m_Session{};
  CAgXSAnOptions m_Options{};
  std::string MakeOptionsString(const CAgXSAnOptions &options) {
    using namespace std::string_literals;
    auto opts = "Simulate="s;
    m_Options = options;
    if (options.Simulate) {
      opts += "true"s;
    } else {
      opts += "false"s;
    }
    if (options.Model != AgXSAnModel::Common) {
      opts += ", DriverSetup=Model="s;
      switch (options.Model) {
        case AgXSAnModel::N9030A:
          opts += "N9030A"s;
          break;
        default:
          opts.clear();
      }
    }
    return opts;
  }

 public:
  auto Connect(const std::string &resource,
               const CAgXSAnOptions &options) noexcept {
    auto optionsString = MakeOptionsString(options);
    return AgXSAn_InitWithOptions(ViRsrc(resource.data()), options.idQuery,
                                  options.Reset, optionsString.data(),
                                  &m_Session);
  }
  void Close() noexcept {
    AgXSAn_close(m_Session);
    m_Session = 0;
  }
  bool IsOpen() const noexcept { return (m_Session != 0); }
  bool IsSimulate() const noexcept { return m_Options.Simulate; }
  ViSession GetSession() const noexcept { return m_Session; }
  SA::CAgXSAnSA const SA{m_Session};
  Input::CAgXSAnInput const Input{m_Session};
  System::CAgXSAnSystem const System{m_Session};
  Acquisition::CAgXSAnAcquisition const Acquisition{m_Session};
  BasicOperation::CAgXSAnBasicOperation const BasicOperation{m_Session};
  Display::CAgXSAnDisplay const Display{m_Session};
  Utility::CAgXSAnUtility const Utility{m_Session};
  Frequency::CAgXSAnFrequency const Frequency{m_Session};
};

}  // namespace AgXSAn

#endif  // AGXSAN_WRAPPER_H
