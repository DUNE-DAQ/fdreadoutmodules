/**
 * @file FDDataHandlerModule.cpp FDDataHandlerModule class implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "FDDataHandlerModule.hpp"

#include "datahandlinglibs/opmon/datahandling_info.pb.h"

#include "logging/Logging.hpp"
#include "iomanager/IOManager.hpp"

#include "datahandlinglibs/DataHandlingIssues.hpp"
#include "datahandlinglibs/ReadoutLogging.hpp"
#include "datahandlinglibs/concepts/DataHandlingConcept.hpp"
#include "datahandlinglibs/models/BinarySearchQueueModel.hpp"
#include "datahandlinglibs/models/DefaultRequestHandlerModel.hpp"
#include "datahandlinglibs/models/EmptyFragmentRequestHandlerModel.hpp"
#include "datahandlinglibs/models/FixedRateQueueModel.hpp"
#include "datahandlinglibs/models/DataHandlingModel.hpp"
#include "datahandlinglibs/models/ZeroCopyRecordingRequestHandlerModel.hpp"
#include "datahandlinglibs/models/DefaultSkipListRequestHandler.hpp"
#include "datahandlinglibs/models/SkipListLatencyBufferModel.hpp"

#include "fdreadoutlibs/DUNEWIBEthTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNESuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNEStreamSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/TDEFrameTypeAdapter.hpp"

#include "fdreadoutlibs/daphne/DAPHNEFrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEStreamFrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEListRequestHandler.hpp"
#include "fdreadoutlibs/wibeth/WIBEthFrameProcessor.hpp"
#include "fdreadoutlibs/tde/TDEEthFrameProcessor.hpp"


#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace dunedaq::datahandlinglibs::logging;

namespace dunedaq {

DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBEthTypeAdapter, "WIBEthFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter, "PDSFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNEStreamSuperChunkTypeAdapter, "PDSStreamFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TDEFrameTypeAdapter, "TDEFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TDEEthTypeAdapter, "TDEEthFrame")

namespace fdreadoutmodules {

FDDataHandlerModule::FDDataHandlerModule(const std::string& name)
  : DAQModule(name)
  , RawDataHandlerBase(name)
{ 

  inherited_mod::register_command("conf", &inherited_dlh::do_conf);
  inherited_mod::register_command("scrap", &inherited_dlh::do_scrap);
  inherited_mod::register_command("start", &inherited_dlh::do_start);
  inherited_mod::register_command("stop_trigger_sources", &inherited_dlh::do_stop);
  inherited_mod::register_command("record", &inherited_dlh::do_record);
}

void
FDDataHandlerModule::init(std::shared_ptr<appfwk::ModuleConfiguration> cfg)
{

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  inherited_dlh::init(cfg);
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

  
 void
 FDDataHandlerModule::generate_opmon_data()
 {
 }

std::shared_ptr<datahandlinglibs::DataHandlingConcept>
FDDataHandlerModule::create_readout(const appmodel::DataHandlerModule* modconf, std::atomic<bool>& run_marker)
{
  namespace rol = dunedaq::datahandlinglibs;
  namespace fdl = dunedaq::fdreadoutlibs;
  namespace fdt = dunedaq::fdreadoutlibs::types;
  

  // Acquire DataType  
  std::string raw_dt = modconf->get_module_configuration()->get_input_data_type();
  TLOG() << "Choosing specializations for DataHandlingModel with data_type:" << raw_dt << ']';

  // IF WIBEth
  if (raw_dt.find("WIBEthFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for an Ethernet DUNE-WIB";
    auto readout_model = std::make_shared<
      rol::DataHandlingModel<fdt::DUNEWIBEthTypeAdapter,
			     rol::ZeroCopyRecordingRequestHandlerModel<fdt::DUNEWIBEthTypeAdapter,
								       rol::FixedRateQueueModel<fdt::DUNEWIBEthTypeAdapter>>,
			     rol::FixedRateQueueModel<fdt::DUNEWIBEthTypeAdapter>,
			     fdl::WIBEthFrameProcessor>>(run_marker);
    register_node("WIBEthFrameProcessor", readout_model);
    readout_model->init(modconf);
    return readout_model;
  }
  
  // IF TDEEth
  if (raw_dt.find("TDEEthFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for an Ethernet TDEEth";
    auto readout_model = 
      std::make_unique<rol::DataHandlingModel<
        fdt::TDEEthTypeAdapter,
        rol::ZeroCopyRecordingRequestHandlerModel<fdt::TDEEthTypeAdapter, rol::FixedRateQueueModel<fdt::TDEEthTypeAdapter>>,
        rol::FixedRateQueueModel<fdt::TDEEthTypeAdapter>,
        fdl::TDEEthFrameProcessor
      >>(run_marker);
    readout_model->init(modconf);
    return readout_model;
  }

  // IF PDS Frame using skiplist
  if (raw_dt.find("PDSFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a PDS DAPHNE using SkipList LB";
    auto readout_model =
      std::make_shared<rol::DataHandlingModel<fdt::DAPHNESuperChunkTypeAdapter,
                                         fdl::DAPHNEListRequestHandler,
                                         rol::SkipListLatencyBufferModel<fdt::DAPHNESuperChunkTypeAdapter>,
                                         fdl::DAPHNEFrameProcessor>>(run_marker);
    register_node("PDSFrameProcessor", readout_model);
    readout_model->init(modconf);
    return readout_model;
  }

  // IF PDS Stream Frame using SPSC LB
  if (raw_dt.find("PDSStreamFrame") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a PDS DAPHNE stream mode using BinarySearchQueue LB";
    auto readout_model = std::make_shared<
      rol::DataHandlingModel<fdt::DAPHNEStreamSuperChunkTypeAdapter,
                        rol::DefaultRequestHandlerModel<fdt::DAPHNEStreamSuperChunkTypeAdapter,
                                                        rol::BinarySearchQueueModel<fdt::DAPHNEStreamSuperChunkTypeAdapter>>,
                        rol::BinarySearchQueueModel<fdt::DAPHNEStreamSuperChunkTypeAdapter>,
                        fdl::DAPHNEStreamFrameProcessor>>(run_marker);
    register_node("PDSStreamFrameProcessor", readout_model);
    readout_model->init(modconf);
    return readout_model;
  }

  return nullptr;
}

} // namespace fdreadoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::fdreadoutmodules::FDDataHandlerModule)
