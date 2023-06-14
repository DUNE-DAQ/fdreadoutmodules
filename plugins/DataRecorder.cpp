/**
 * @file DataRecorder.cpp DataRecorder implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
//#include "readout/NDReadoutTypes.hpp"

//#include "fdreadoutlibs/ProtoWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBEthTypeAdapter.hpp"
#include "fdreadoutlibs/TDEFrameTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNESuperChunkTypeAdapter.hpp"
//#include "ndreadoutlibs/NDReadoutPACMANTypeAdapter.hpp"
//#include "ndreadoutlibs/NDReadoutMPDTypeAdapter.hpp"

#include "readoutlibs/ReadoutLogging.hpp"
#include "readoutlibs/models/RecorderModel.hpp"
#include "readoutlibs/recorderconfig/Nljs.hpp"
#include "readoutlibs/recorderconfig/Structs.hpp"
#include "readoutlibs/recorderinfo/InfoNljs.hpp"

#include "DataRecorder.hpp"
#include "appfwk/DAQModuleHelper.hpp"

#include "appfwk/cmd/Nljs.hpp"
#include "logging/Logging.hpp"
#include "readoutlibs/ReadoutIssues.hpp"
#include <string>

using namespace dunedaq::readoutlibs::logging;

namespace dunedaq {
namespace fdreadoutmodules {

DataRecorder::DataRecorder(const std::string& name)
  : DAQModule(name)
{
  register_command("conf", &DataRecorder::do_conf);
  register_command("scrap", &DataRecorder::do_scrap);
  register_command("start", &DataRecorder::do_start);
  register_command("stop_trigger_sources", &DataRecorder::do_stop);
}

void
DataRecorder::init(const data_t& args)
{
  try {
    // Acquire input connection and its DataType
    auto ci = appfwk::connection_index(args, {"raw_recording"});
    auto datatypes = dunedaq::iomanager::IOManager::get()->get_datatypes(ci["raw_recording"]);
    if (datatypes.size() != 1) {
    ers::error(dunedaq::readoutlibs::GenericConfigurationError(ERS_HERE,
      "Multiple raw_recording queues specified! Expected only a single raw_dtance!"));
    }
    std::string raw_dt{ *datatypes.begin() };
    TLOG() << "Choosing specializations for RecorderModel with raw_recording"
           << " [uid:" << ci["raw_recording"] << " , data_type:" << raw_dt << ']';

    // IF WIB2
    if (raw_dt.find("WIB2Frame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for wib2";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter>(get_name()));
      recorder->init(args);
      return;
    }

    /* IF WIB
    if (raw_dt.find("WIBFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for wib";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter>(get_name()));
      recorder->init(args);
      return;
    }
    */

    // IF WIBEth
    if (raw_dt.find("WIBEthFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for wibeth";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::DUNEWIBEthTypeAdapter>(get_name()));
      recorder->init(args);
      return;
    }

    // IF PDS
    if (raw_dt.find("PDSFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for pds";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter>(get_name()));
      recorder->init(args);
      return;
    }

    // IF PACMAN
    /*
      if (raw_dt.find("PACMANFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for pacman";
      recorder.reset(new readoutlibs::RecorderModel<ndreadoutlibs::types::NDReadoutPACMANTypeAdapter>(get_name()));
      recorder->init(args);
      return;
    }

    // IF MPD
    if (raw_dt.find("MPDFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for mpd";
      recorder.reset(new readoutlibs::RecorderModel<ndreadoutlibs::types::NDReadoutMPDTypeAdapter>(get_name()));
      recorder->init(args);
      return;
    }
    */

    // IF TDE
    if (raw_dt.find("TDEFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for tde";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::TDEFrameTypeAdapter>(get_name()));
      recorder->init(args);
      return;
    }

    throw readoutlibs::DataRecorderConfigurationError(ERS_HERE, "Could not create DataRecorder of type " + raw_dt);

  } catch (const ers::Issue& excpt) {
    throw readoutlibs::DataRecorderResourceQueueError(ERS_HERE, "Could not initialize queue", "raw_recording", "");
  }
}

void
DataRecorder::get_info(opmonlib::InfoCollector& ci, int level)
{
  recorder->get_info(ci, level);
}

void
DataRecorder::do_conf(const data_t& args)
{
  recorder->do_conf(args);
}

void
DataRecorder::do_scrap(const data_t& args)
{
  recorder->do_scrap(args);
}

void
DataRecorder::do_start(const data_t& args)
{
  recorder->do_start(args);
}

void
DataRecorder::do_stop(const data_t& args)
{
  recorder->do_stop(args);
}

} // namespace fdreadoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::fdreadoutmodules::DataRecorder)
