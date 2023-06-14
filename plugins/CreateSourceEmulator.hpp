/**
 * @file CreateSourceEmulator.hpp Specific source emulator creator.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_SRC_CREATESOURCEEMULATOR_HPP_
#define READOUTMODULES_SRC_CREATESOURCEEMULATOR_HPP_

#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/cmd/Structs.hpp"

#include "readoutlibs/ReadoutLogging.hpp"
#include "readoutlibs/models/SourceEmulatorModel.hpp"

//#include "fdreadoutlibs/ProtoWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBEthTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNESuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/TDEFrameTypeAdapter.hpp"
//#include "fdreadoutlibs/TriggerPrimitiveTypeAdapter.hpp"

#include <memory>
#include <string>
#include <utility>

using dunedaq::readoutlibs::logging::TLVL_WORK_STEPS;

namespace dunedaq {

//DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter, "WIBFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter, "WIB2Frame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBEthTypeAdapter, "WIBEthFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter, "PDSFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TDEFrameTypeAdapter, "TDEFrame")
//DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TriggerPrimitiveTypeAdapter, "TriggerPrimitive")

namespace fdreadoutmodules {

std::unique_ptr<readoutlibs::SourceEmulatorConcept>
createSourceEmulator(const appfwk::app::ConnectionReference qi, std::atomic<bool>& run_marker)
{
  //! Values suitable to emulation

  static constexpr int daphne_time_tick_diff = 16;
  static constexpr double daphne_dropout_rate = 0.9;
  static constexpr double daphne_rate_khz = 200.0;
  static constexpr int daphne_frames_per_tick = 1;

  static constexpr int wib_time_tick_diff = 25;
  static constexpr double wib_dropout_rate = 0.0;
  static constexpr double wib_rate_khz = 166.0;
  static constexpr int wib_frames_per_tick = 1;

  static constexpr int wib2_time_tick_diff = 32;
  static constexpr double wib2_dropout_rate = 0.0;
  static constexpr double wib2_rate_khz = 166.0;
  static constexpr int wib2_frames_per_tick = 1;

  static constexpr int wibeth_time_tick_diff = 32*64;
  static constexpr double wibeth_dropout_rate = 0.0;
  static constexpr double wibeth_rate_khz = 30.5176;
  static constexpr int wibeth_frames_per_tick = 1;

  static constexpr int tde_time_tick_diff = dunedaq::fddetdataformats::ticks_between_adc_samples*dunedaq::fddetdataformats::tot_adc16_samples;
  static constexpr double tde_dropout_rate = 0.0;
  static constexpr double tde_rate_khz = 62500./tde_time_tick_diff;
  static constexpr int tde_frames_per_tick = dunedaq::fddetdataformats::n_channels_per_amc;

  static constexpr double emu_frame_error_rate = 0.0;

  auto datatypes = dunedaq::iomanager::IOManager::get()->get_datatypes(qi.uid);
  if (datatypes.size() != 1) {
    ers::error(dunedaq::readoutlibs::GenericConfigurationError(ERS_HERE,
      "Multiple output data types specified! Expected only a single type!"));
  }
  std::string raw_dt{ *datatypes.begin() };
  TLOG() << "Choosing specialization for SourceEmulator with raw_input"
         << " [uid:" << qi.uid << " , data_type:" << raw_dt << ']';

  // IF WIBETH
  if (raw_dt.find("WIBEthFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake wibeth link";
    auto source_emu_model =
      std::make_unique<readoutlibs::SourceEmulatorModel<fdreadoutlibs::types::DUNEWIBEthTypeAdapter>>(
        qi.name, run_marker, wibeth_time_tick_diff, wibeth_dropout_rate, emu_frame_error_rate, wibeth_rate_khz, wibeth_frames_per_tick);
    return source_emu_model;
  }

  // IF WIB2
  if (raw_dt.find("WIB2Frame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake wib2 link";

    auto source_emu_model =
      std::make_unique<readoutlibs::SourceEmulatorModel<fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter>>(
        qi.name, run_marker, wib2_time_tick_diff, wib2_dropout_rate, emu_frame_error_rate, wib2_rate_khz, wib2_frames_per_tick);
    return source_emu_model;
  }

  /* IF WIB
  if (raw_dt.find("WIBFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake wib link";
    auto source_emu_model =
      std::make_unique<readoutlibs::SourceEmulatorModel<fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter>>(
        qi.name, run_marker, wib_time_tick_diff, wib_dropout_rate, emu_frame_error_rate, wib_rate_khz, wib_frames_per_tick);
    return source_emu_model;
  }
  */
  // IF PDS
  if (raw_dt.find("PDSFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake pds link";
    auto source_emu_model =
      std::make_unique<readoutlibs::SourceEmulatorModel<fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter>>(
        qi.name, run_marker, daphne_time_tick_diff, daphne_dropout_rate, emu_frame_error_rate, daphne_rate_khz, daphne_frames_per_tick);
    return source_emu_model;
  }

  // IF TDE
  if (raw_dt.find("TDEFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake tde link";
    auto source_emu_model =
      std::make_unique<readoutlibs::SourceEmulatorModel<fdreadoutlibs::types::TDEFrameTypeAdapter>>(
        qi.name, run_marker, tde_time_tick_diff, tde_dropout_rate, emu_frame_error_rate, tde_rate_khz, tde_frames_per_tick);
    return source_emu_model;
  }

  return nullptr;
}

} // namespace fdreadoutmodules
} // namespace dunedaq

#endif // FDREADOUTMODULES_SRC_CREATESOURCEEMULATOR_HPP_
