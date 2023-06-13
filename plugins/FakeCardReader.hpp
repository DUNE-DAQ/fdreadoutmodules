/**
 * @file FakeCardReader.hpp FELIX FULLMODE Fake Links
 * Generates user payloads at a given rate, from WIB to FELIX binary captures
 * This implementation is purely software based, no FELIX card and tools
 * are needed to use this module.
 *
 * TODO: Make it generic, to consumer other types of user payloads.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FDREADOUTMODULES_PLUGINS_FAKECARDREADER_HPP_
#define FDREADOUTMODULES_PLUGINS_FAKECARDREADER_HPP_

// package

//#include "fdreadoutlibs/ProtoWIBSuperChunkTypeAdapter.hpp"
#include "readoutlibs/concepts/SourceEmulatorConcept.hpp"
#include "readoutlibs/sourceemulatorconfig/Structs.hpp"
#include "readoutlibs/utils/ReusableThread.hpp"
//#include "CreateSourceEmulator.hpp"
#include "readoutlibs/utils/FileSourceBuffer.hpp"

// appfwk
#include "appfwk/DAQModule.hpp"
#include "iomanager/IOManager.hpp"

// std
#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {
namespace fdreadoutmodules {

class FakeCardReader : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief FakeCardReader Constructor
   * @param name Instance name for this FakeCardReader instance
   */
  explicit FakeCardReader(const std::string& name);

  FakeCardReader(const FakeCardReader&) = delete;            ///< FakeCardReader is not copy-constructible
  FakeCardReader& operator=(const FakeCardReader&) = delete; ///< FakeCardReader is not copy-assignable
  FakeCardReader(FakeCardReader&&) = delete;                 ///< FakeCardReader is not move-constructible
  FakeCardReader& operator=(FakeCardReader&&) = delete;      ///< FakeCardReader is not move-assignable

  void init(const data_t&) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

private:
  //using sink_t = iomanager::SenderConcept<fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter>;
  // Commands
  void do_conf(const data_t& /*args*/);
  void do_scrap(const data_t& /*args*/);
  void do_start(const data_t& /*args*/);
  void do_stop(const data_t& /*args*/);

  //void generate_data(sink_t* queue, int link_id);

  // Configuration
  bool m_configured;
  using module_conf_t = readoutlibs::sourceemulatorconfig::Conf;
  module_conf_t m_cfg;

  std::map<std::string, std::unique_ptr<readoutlibs::SourceEmulatorConcept>> m_source_emus;

  // data senders
  //std::vector<sink_t*> m_data_senders;

  // Internals
  std::unique_ptr<readoutlibs::FileSourceBuffer> m_source_buffer;

  // Threading
  std::atomic<bool> m_run_marker;
};

} // namespace fdreadoutmodules
} // namespace dunedaq

#endif // FDREADOUTMODULES_PLUGINS_FAKECARDREADER_HPP_
