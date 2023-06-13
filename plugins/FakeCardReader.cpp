/**
 * @file FakeCardReader.cpp FakeCardReader class implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "readoutlibs/ReadoutLogging.hpp"
#include "readoutlibs/sourceemulatorconfig/Nljs.hpp"


#include "FakeCardReader.hpp"
#include "readoutlibs/ReadoutIssues.hpp"
#include "readoutlibs/models/SourceEmulatorModel.hpp"

#include "CreateSourceEmulator.hpp"

#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"
//#include "fddetdataformats/WIBFrame.hpp"
//#include "detdataformats/wib/RawWIBTp.hpp"
//#include "fddetdataformats/TDE16Frame.hpp"
#include "logging/Logging.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace dunedaq::readoutlibs::logging;

namespace dunedaq {
namespace fdreadoutmodules {

FakeCardReader::FakeCardReader(const std::string& name)
  : DAQModule(name)
  , m_configured(false)
  , m_run_marker{ false }
{
  register_command("conf", &FakeCardReader::do_conf);
  register_command("scrap", &FakeCardReader::do_scrap);
  register_command("start", &FakeCardReader::do_start);
  register_command("drain_dataflow", &FakeCardReader::do_stop);
}

void
FakeCardReader::init(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  auto ini = args.get<appfwk::app::ModInit>();
  for (const auto& qi : ini.conn_refs) {
    
    try {
      if (m_source_emus.find(qi.name) != m_source_emus.end()) {
        TLOG() << get_name() << "Same queue instance used twice";
        throw readoutlibs::FailedFakeCardInitialization(ERS_HERE, get_name(), args.dump());
      }
      m_source_emus[qi.name] = createSourceEmulator(qi, m_run_marker);
      if (m_source_emus[qi.name].get() == nullptr) {
        TLOG() << get_name() << "Source emulator could not be created";
        throw readoutlibs::FailedFakeCardInitialization(ERS_HERE, get_name(), args.dump());
      }
      m_source_emus[qi.name]->init(args);
      m_source_emus[qi.name]->set_sender(qi.uid);
    } catch (const ers::Issue& excpt) {
      throw readoutlibs::ResourceQueueError(ERS_HERE, qi.name, get_name(), excpt);
    }
  }
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

void
FakeCardReader::get_info(opmonlib::InfoCollector& ci, int level)
{

  for (auto& [name, emu] : m_source_emus) {
    emu->get_info(ci, level);
  }
}

void
FakeCardReader::do_conf(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_conf() method";

  if (m_configured) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "This module is already configured!";
  } else {
    m_cfg = args.get<readoutlibs::sourceemulatorconfig::Conf>();

    for (const auto& emu_conf : m_cfg.link_confs) {
      if (m_source_emus.find(emu_conf.queue_name) == m_source_emus.end()) {
        TLOG() << "Cannot find queue: " << emu_conf.queue_name << std::endl;
        throw readoutlibs::GenericConfigurationError(ERS_HERE, "Cannot find queue: " + emu_conf.queue_name);
      }
      if (m_source_emus[emu_conf.queue_name]->is_configured()) {
        TLOG() << "Emulator for queue name " << emu_conf.queue_name << " was already configured";
        throw readoutlibs::GenericConfigurationError(ERS_HERE, "Emulator configured twice: " + emu_conf.queue_name);
      }
      m_source_emus[emu_conf.queue_name]->conf(args, emu_conf);
    }

    for (auto& [name, emu] : m_source_emus) {
      if (!emu->is_configured()) {
        throw readoutlibs::GenericConfigurationError(ERS_HERE, "Not all links were configured");
      }
    }

    // Mark configured
    m_configured = true;
  }

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_conf() method";
}

void
FakeCardReader::do_scrap(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_scrap() method";

  for (auto& [name, emu] : m_source_emus) {
    emu->scrap(args);
  }

  m_configured = false;

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_scrap() method";
}
void
FakeCardReader::do_start(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_start() method";

  m_run_marker.store(true);

  for (auto& [name, emu] : m_source_emus) {
    emu->start(args);
  }

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_start() method";
}

void
FakeCardReader::do_stop(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_stop() method";

  m_run_marker = false;

  for (auto& [name, emu] : m_source_emus) {
    emu->stop(args);
  }

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_stop() method";
}

} // namespace readoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::fdreadoutmodules::FakeCardReader)
