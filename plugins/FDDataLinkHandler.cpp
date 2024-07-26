/**
 * @file FDDataLinkHandler.cpp FDDataLinkHandler class implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "FDDataLinkHandler.hpp"

#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Structs.hpp"

#include "logging/Logging.hpp"
#include "iomanager/IOManager.hpp"

#include "readoutlibs/ReadoutIssues.hpp"
#include "readoutlibs/ReadoutLogging.hpp"
#include "readoutlibs/concepts/ReadoutConcept.hpp"
#include "readoutlibs/models/BinarySearchQueueModel.hpp"
#include "readoutlibs/models/DefaultRequestHandlerModel.hpp"
#include "readoutlibs/models/EmptyFragmentRequestHandlerModel.hpp"
#include "readoutlibs/models/FixedRateQueueModel.hpp"
#include "readoutlibs/models/ReadoutModel.hpp"
#include "readoutlibs/models/ZeroCopyRecordingRequestHandlerModel.hpp"
#include "readoutlibs/models/DefaultSkipListRequestHandler.hpp"
#include "readoutlibs/models/SkipListLatencyBufferModel.hpp"

//#include "fdreadoutlibs/ProtoWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBEthTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNESuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNEStreamSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/SSPFrameTypeAdapter.hpp"
#include "fdreadoutlibs/TDEFrameTypeAdapter.hpp"
#include "fdreadoutlibs/TriggerPrimitiveTypeAdapter.hpp"
#include "fdreadoutlibs/CRTTypeAdapter.hpp"

#include "fdreadoutlibs/daphne/DAPHNEFrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEStreamFrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEListRequestHandler.hpp"
#include "fdreadoutlibs/ssp/SSPFrameProcessor.hpp"
//#include "fdreadoutlibs/wib2/SWWIB2TriggerPrimitiveProcessor.hpp"
#include "fdreadoutlibs/wib2/WIB2FrameProcessor.hpp"
#include "fdreadoutlibs/TPCTPRequestHandler.hpp"
#include "fdreadoutlibs/wibeth/WIBEthFrameProcessor.hpp"
#include "fdreadoutlibs/tde/TDEFrameProcessor.hpp"
#include "fdreadoutlibs/crt/CRTFrameProcessor.hpp"
//#include "fdreadoutlibs/wib/WIBFrameProcessor.hpp"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace dunedaq::readoutlibs::logging;

namespace dunedaq {

//DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter, "WIBFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter, "WIB2Frame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBEthTypeAdapter, "WIBEthFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter, "PDSFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNEStreamSuperChunkTypeAdapter, "PDSStreamFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::SSPFrameTypeAdapter, "SSPFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TDEFrameTypeAdapter, "TDEFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::CRTTypeAdapter, "CRTFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TriggerPrimitiveTypeAdapter, "TriggerPrimitive")
//DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBFirmwareTriggerPrimitiveSuperChunkTypeAdapter, "FWTriggerPrimitive")

namespace fdreadoutmodules {

FDDataLinkHandler::FDDataLinkHandler(const std::string& name)
  : DAQModule(name)
  , DataLinkHandlerBase(name)
{ 
  //inherited_dlh::m_readout_creator = make_readout_creator("fd");

  inherited_mod::register_command("conf", &inherited_dlh::do_conf);
  inherited_mod::register_command("scrap", &inherited_dlh::do_scrap);
  inherited_mod::register_command("start", &inherited_dlh::do_start);
  inherited_mod::register_command("stop_trigger_sources", &inherited_dlh::do_stop);
  inherited_mod::register_command("record", &inherited_dlh::do_record);
}

void
FDDataLinkHandler::init(const data_t& args)
{

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  inherited_dlh::init(args);
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

void
FDDataLinkHandler::get_info(opmonlib::InfoCollector& ci, int level)
{
  inherited_dlh::get_info(ci, level);
}

std::unique_ptr<readoutlibs::ReadoutConcept>
FDDataLinkHandler::create_readout(const nlohmann::json& args, std::atomic<bool>& run_marker)
{
  namespace rol = dunedaq::readoutlibs;
  namespace fdl = dunedaq::fdreadoutlibs;
  namespace fdt = dunedaq::fdreadoutlibs::types;

  // Acquire input connection and its DataType
  auto ci = appfwk::connection_index(args, {"raw_input"});
  auto datatypes = dunedaq::iomanager::IOManager::get()->get_datatypes(ci["raw_input"]);
  if (datatypes.size() != 1) {
    ers::error(dunedaq::readoutlibs::GenericConfigurationError(ERS_HERE,
      "Multiple raw_input queues specified! Expected only a single instance!"));
  }
  std::string raw_dt{ *datatypes.begin() };
  TLOG() << "Choosing specializations for ReadoutModel with raw_input"
         << " [uid:" << ci["raw_input"] << " , data_type:" << raw_dt << ']';

  // Chose readout specializations based on DataType
  /* IF WIB
  if (raw_dt.find("WIBFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for ProtoDUNE-WIB";
    auto readout_model = std::make_unique<rol::ReadoutModel<
      fdt::ProtoWIBSuperChunkTypeAdapter,
      rol::ZeroCopyRecordingRequestHandlerModel<fdt::ProtoWIBSuperChunkTypeAdapter,
                                                rol::FixedRateQueueModel<fdt::ProtoWIBSuperChunkTypeAdapter>>,
      rol::FixedRateQueueModel<fdt::ProtoWIBSuperChunkTypeAdapter>,
      fdl::WIBFrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }
*/
  /*
  // IF WIB2
  if (raw_dt.find("WIB2Frame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a DUNE-WIB";
    auto readout_model = std::make_unique<
      rol::ReadoutModel<fdt::DUNEWIBSuperChunkTypeAdapter,
                        rol::DefaultRequestHandlerModel<fdt::DUNEWIBSuperChunkTypeAdapter,
                                                        rol::FixedRateQueueModel<fdt::DUNEWIBSuperChunkTypeAdapter>>,
                        rol::FixedRateQueueModel<fdt::DUNEWIBSuperChunkTypeAdapter>,
                        fdl::WIB2FrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }
  */
  // IF WIB2
  if (raw_dt.find("WIB2Frame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a DUNE-WIB";
    auto readout_model = std::make_unique<
      rol::ReadoutModel<fdt::DUNEWIBSuperChunkTypeAdapter,
                        rol::ZeroCopyRecordingRequestHandlerModel<fdt::DUNEWIBSuperChunkTypeAdapter,
                                                        rol::FixedRateQueueModel<fdt::DUNEWIBSuperChunkTypeAdapter>>,
                        rol::FixedRateQueueModel<fdt::DUNEWIBSuperChunkTypeAdapter>,
                        fdl::WIB2FrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }
  /*
  // IF WIBEth
  if (raw_dt.find("WIBEthFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for an Ethernet DUNE-WIB";
    auto readout_model = std::make_unique<
      rol::ReadoutModel<fdt::DUNEWIBEthTypeAdapter,
                        rol::DefaultRequestHandlerModel<fdt::DUNEWIBEthTypeAdapter,
                                                        rol::FixedRateQueueModel<fdt::DUNEWIBEthTypeAdapter>>,
                        rol::FixedRateQueueModel<fdt::DUNEWIBEthTypeAdapter>,
                        fdl::WIBEthFrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }
  */

  // IF WIBEth
  if (raw_dt.find("WIBEthFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for an Ethernet DUNE-WIB";
    auto readout_model = std::make_unique<
      rol::ReadoutModel<fdt::DUNEWIBEthTypeAdapter,
			rol::ZeroCopyRecordingRequestHandlerModel<fdt::DUNEWIBEthTypeAdapter,
                                                        rol::FixedRateQueueModel<fdt::DUNEWIBEthTypeAdapter>>,
                        rol::FixedRateQueueModel<fdt::DUNEWIBEthTypeAdapter>,
                        fdl::WIBEthFrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }
  
  // IF DAPHNE but use of SPSC queues as LB
  //if (inst.find("pds_queue") != std::string::npos) {
  //  TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pds using Searchable Queue";
  //  auto readout_model = std::make_unique<
  //    rol::ReadoutModel<fdt::DAPHNESuperChunkTypeAdapter,
  //                      rol::DefaultRequestHandlerModel<fdt::DAPHNESuperChunkTypeAdapter,
  //                                                      rol::BinarySearchQueueModel<fdt::DAPHNESuperChunkTypeAdapter>>,
  //                      rol::BinarySearchQueueModel<fdt::DAPHNESuperChunkTypeAdapter>,
  //                      fdl::DAPHNEFrameProcessor>>(run_marker);
  //  readout_model->init(args);
  //  return readout_model;
  //}

  // IF PDS Frame using skiplist
  if (raw_dt.find("PDSFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a PDS DAPHNE using SkipList LB";
    auto readout_model =
      std::make_unique<rol::ReadoutModel<fdt::DAPHNESuperChunkTypeAdapter,
                                         fdl::DAPHNEListRequestHandler,
                                         rol::SkipListLatencyBufferModel<fdt::DAPHNESuperChunkTypeAdapter>,
                                         fdl::DAPHNEFrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }

  // IF PDS Stream Frame using SPSC LB
  if (raw_dt.find("PDSStreamFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a PDS DAPHNE stream mode using BinarySearchQueue LB";
    auto readout_model = std::make_unique<
      rol::ReadoutModel<fdt::DAPHNEStreamSuperChunkTypeAdapter,
                        rol::DefaultRequestHandlerModel<fdt::DAPHNEStreamSuperChunkTypeAdapter,
                                                        rol::BinarySearchQueueModel<fdt::DAPHNEStreamSuperChunkTypeAdapter>>,
                        rol::BinarySearchQueueModel<fdt::DAPHNEStreamSuperChunkTypeAdapter>,
                        fdl::DAPHNEStreamFrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }

  // IF SSP
  if (raw_dt.find("SSPFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a SSPs using BinarySearchQueue LB";
    auto readout_model = std::make_unique<rol::ReadoutModel<
      fdt::SSPFrameTypeAdapter,
      rol::DefaultRequestHandlerModel<fdt::SSPFrameTypeAdapter, rol::BinarySearchQueueModel<fdt::SSPFrameTypeAdapter>>,
      rol::BinarySearchQueueModel<fdt::SSPFrameTypeAdapter>,
      fdl::SSPFrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }

  // If TDE
  if (raw_dt.find("TDEFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for TDE";
    auto readout_model = std::make_unique<
      rol::ReadoutModel<fdt::TDEFrameTypeAdapter,
                        rol::DefaultSkipListRequestHandler<fdt::TDEFrameTypeAdapter>,
                        rol::SkipListLatencyBufferModel<fdt::TDEFrameTypeAdapter>,
                        fdl::TDEFrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }

  // IF CRT
  if (raw_dt.find("CRTFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a CRT";
    auto readout_model = std::make_unique<
      rol::ReadoutModel<fdt::CRTTypeAdapter,
                        rol::DefaultSkipListRequestHandler<fdt::CRTTypeAdapter>,
                        rol::SkipListLatencyBufferModel<fdt::CRTTypeAdapter>,
                        fdl::CRTFrameProcessor>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }

  // IF TriggerPrimitive (TPG)
  if (raw_dt.find("TriggerPrimitive") != std::string::npos) {
    TLOG(TLVL_WORK_STEPS) << "Creating readout for TriggerPrimitive";
    auto readout_model = std::make_unique<rol::ReadoutModel<
      fdt::TriggerPrimitiveTypeAdapter,
      fdl::TPCTPRequestHandler,
      rol::SkipListLatencyBufferModel<fdt::TriggerPrimitiveTypeAdapter>,
      rol::TaskRawDataProcessorModel<fdt::TriggerPrimitiveTypeAdapter>>>(run_marker);
    readout_model->init(args);
    return readout_model;
  }

  return nullptr;
}

} // namespace fdreadoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::fdreadoutmodules::FDDataLinkHandler)
