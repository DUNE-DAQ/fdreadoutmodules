/**
 * @file FDFakeReaderModule.hpp FarDetector FakeCardReader
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FDREADOUTMODULES_PLUGINS_FDFAKECARDREADER_HPP_
#define FDREADOUTMODULES_PLUGINS_FDFAKECARDREADER_HPP_

//#include "appfwk/cmd/Nljs.hpp"
//#include "appfwk/app/Nljs.hpp"
//#include "appfwk/cmd/Structs.hpp"

#include "appfwk/DAQModule.hpp"
#include "appfwk/ModuleConfiguration.hpp"

#include "datahandlinglibs/FakeCardReaderBase.hpp"

#include <string>

namespace dunedaq {
namespace fdreadoutmodules {

class FDFakeReaderModule : public dunedaq::appfwk::DAQModule,
                         public dunedaq::datahandlinglibs::FakeCardReaderBase
{
public:
  using inherited_fcr = dunedaq::datahandlinglibs::FakeCardReaderBase;
  using inherited_mod = dunedaq::appfwk::DAQModule;
  /**
   * @brief FDFakeReaderModule Constructor
   * @param name Instance name for this FDFakeReaderModule instance
   */
  explicit FDFakeReaderModule(const std::string& name);

  FDFakeReaderModule(const FDFakeReaderModule&) = delete;            ///< FDFakeReaderModule is not copy-constructible
  FDFakeReaderModule& operator=(const FDFakeReaderModule&) = delete; ///< FDFakeReaderModule is not copy-assignable
  FDFakeReaderModule(FDFakeReaderModule&&) = delete;                 ///< FDFakeReaderModule is not move-constructible
  FDFakeReaderModule& operator=(FDFakeReaderModule&&) = delete;      ///< FDFakeReaderModule is not move-assignable

  void init(std::shared_ptr<appfwk::ModuleConfiguration> cfg) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

  std::unique_ptr<datahandlinglibs::SourceEmulatorConcept>
  create_source_emulator(std::string qi, std::atomic<bool>& run_marker) override;

};

} // namespace fdreadoutmodules
} // namespace dunedaq

#endif // FDREADOUTMODULES_PLUGINS_FDFAKECARDREADER_HPP_
