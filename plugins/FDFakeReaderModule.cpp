/**
 * @file FDFakeReaderModule.cpp FDFakeReaderModule class implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "FDFakeReaderModule.hpp"

//#include "appfwk/app/Nljs.hpp"
//#include "appfwk/cmd/Nljs.hpp"
#include "logging/Logging.hpp"

#include "datahandlinglibs/ReadoutLogging.hpp"
#include "datahandlinglibs/DataHandlingIssues.hpp"
//#include "datahandlinglibs/sourceemulatorconfig/Nljs.hpp"
#include "datahandlinglibs/models/SourceEmulatorModel.hpp"
#include "appmodel/DataReaderModule.hpp"

//#include "fdreadoutlibs/DUNEWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBEthTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNESuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNEStreamSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/TDEFrameTypeAdapter.hpp"
#include "fdreadoutlibs/TDEEthTypeAdapter.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace dunedaq::datahandlinglibs::logging;

namespace dunedaq {

//DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter, "WIB2Frame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBEthTypeAdapter, "WIBEthFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter, "PDSFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNEStreamSuperChunkTypeAdapter, "PDSStreamFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TDEFrameTypeAdapter, "TDEFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TDEEthTypeAdapter, "TDEEthFrame")

namespace fdreadoutmodules {

FDFakeReaderModule::FDFakeReaderModule(const std::string& name)
  : DAQModule(name)
  , FakeCardReaderBase(name)
{
  inherited_mod::register_command("conf", &inherited_fcr::do_conf);
  inherited_mod::register_command("scrap", &inherited_fcr::do_scrap);
  inherited_mod::register_command("start", &inherited_fcr::do_start);
  inherited_mod::register_command("stop_trigger_sources", &inherited_fcr::do_stop);
}

void
FDFakeReaderModule::init(std::shared_ptr<appfwk::ModuleConfiguration> cfg)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  inherited_fcr::init(cfg);
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

std::shared_ptr<datahandlinglibs::SourceEmulatorConcept>
FDFakeReaderModule::create_source_emulator(std::string q_id, std::atomic<bool>& run_marker)
{
  //! Values suitable to emulation

  static constexpr int daphne_time_tick_diff = 16;
  static constexpr double daphne_dropout_rate = 0.9;
  static constexpr double daphne_rate_khz = 200.0;
  static constexpr int daphne_frames_per_tick = 1;

  static constexpr int wibeth_time_tick_diff = 32*64;
  static constexpr double wibeth_dropout_rate = 0.0;
  static constexpr double wibeth_rate_khz = 30.5176;
  static constexpr int wibeth_frames_per_tick = 1;

  static constexpr int tde_time_tick_diff = dunedaq::fddetdataformats::ticks_between_adc_samples*dunedaq::fddetdataformats::tot_adc16_samples;
  static constexpr double tde_dropout_rate = 0.0;
  static constexpr double tde_rate_khz = 62500./tde_time_tick_diff;
  static constexpr int tde_frames_per_tick = dunedaq::fddetdataformats::n_channels_per_amc;

  static constexpr double emu_frame_error_rate = 0.0;

  auto datatypes = dunedaq::iomanager::IOManager::get()->get_datatypes(q_id);
  if (datatypes.size() != 1) {
    ers::error(dunedaq::datahandlinglibs::GenericConfigurationError(ERS_HERE,
      "Multiple output data types specified! Expected only a single type!"));
  }
  std::string raw_dt{ *datatypes.begin() };
  TLOG() << "Choosing specialization for SourceEmulator with raw_input"
         << " [uid:" << q_id << " , data_type:" << raw_dt << ']';

  // IF WIBETH
  if (raw_dt.find("WIBEthFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake wibeth link";
    auto source_emu_model =
      std::make_shared<datahandlinglibs::SourceEmulatorModel<fdreadoutlibs::types::DUNEWIBEthTypeAdapter>>(
        q_id, run_marker, wibeth_time_tick_diff, wibeth_dropout_rate, emu_frame_error_rate, wibeth_rate_khz, wibeth_frames_per_tick);
    register_node(q_id, source_emu_model);
    return source_emu_model;
  }

  // IF PDS
  if (raw_dt.find("PDSFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake pds link";
    auto source_emu_model =
      std::make_shared<datahandlinglibs::SourceEmulatorModel<fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter>>(
        q_id, run_marker, daphne_time_tick_diff, daphne_dropout_rate, emu_frame_error_rate, daphne_rate_khz, daphne_frames_per_tick);
      register_node(q_id, source_emu_model);
      return source_emu_model;
  }

  // IF PDSStream
  if (raw_dt.find("PDSStreamFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake pds stream link";
    auto source_emu_model =
      std::make_shared<datahandlinglibs::SourceEmulatorModel<fdreadoutlibs::types::DAPHNEStreamSuperChunkTypeAdapter>>(
        q_id, run_marker, daphne_time_tick_diff, daphne_dropout_rate, emu_frame_error_rate, daphne_rate_khz, daphne_frames_per_tick);
      register_node(q_id, source_emu_model);
    return source_emu_model;
  }

  // IF TDE
  if (raw_dt.find("TDEFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake tde link";
    auto source_emu_model =
      std::make_shared<datahandlinglibs::SourceEmulatorModel<fdreadoutlibs::types::TDEFrameTypeAdapter>>(
        q_id, run_marker, tde_time_tick_diff, tde_dropout_rate, emu_frame_error_rate, tde_rate_khz, tde_frames_per_tick);
      register_node(q_id, source_emu_model);
    return source_emu_model;
  }

  // IF TDEEth
  if (raw_dt.find("TDEEthFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake tde link";
    auto source_emu_model =
      std::make_shared<datahandlinglibs::SourceEmulatorModel<fdreadoutlibs::types::TDEEthTypeAdapter>>(
        q_id,
        run_marker,
        tde_time_tick_diff,
        tde_dropout_rate,
        emu_frame_error_rate,
        tde_rate_khz,
        tde_frames_per_tick);
    register_node(q_id, source_emu_model);
    return source_emu_model;
  }


  return nullptr;
}

} // namespace fdreadoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::fdreadoutmodules::FDFakeReaderModule)
