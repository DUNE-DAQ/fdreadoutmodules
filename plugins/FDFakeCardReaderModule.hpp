/**
 * @file FDFakeCardReaderModule.hpp FarDetector FakeCardReader
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

#include "readoutmodules/FakeCardReaderBase.hpp"

#include <string>

namespace dunedaq {
namespace fdreadoutmodules {

class FDFakeCardReaderModule : public dunedaq::appfwk::DAQModule,
                         public dunedaq::readoutmodules::FakeCardReaderBase
{
public:
  using inherited_fcr = dunedaq::readoutmodules::FakeCardReaderBase;
  using inherited_mod = dunedaq::appfwk::DAQModule;
  /**
   * @brief FDFakeCardReaderModule Constructor
   * @param name Instance name for this FDFakeCardReaderModule instance
   */
  explicit FDFakeCardReaderModule(const std::string& name);

  FDFakeCardReaderModule(const FDFakeCardReaderModule&) = delete;            ///< FDFakeCardReaderModule is not copy-constructible
  FDFakeCardReaderModule& operator=(const FDFakeCardReaderModule&) = delete; ///< FDFakeCardReaderModule is not copy-assignable
  FDFakeCardReaderModule(FDFakeCardReaderModule&&) = delete;                 ///< FDFakeCardReaderModule is not move-constructible
  FDFakeCardReaderModule& operator=(FDFakeCardReaderModule&&) = delete;      ///< FDFakeCardReaderModule is not move-assignable

  void init(std::shared_ptr<appfwk::ModuleConfiguration> cfg) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

  std::unique_ptr<readoutlibs::SourceEmulatorConcept>
  create_source_emulator(std::string qi, std::atomic<bool>& run_marker) override;

};

} // namespace fdreadoutmodules
} // namespace dunedaq

#endif // FDREADOUTMODULES_PLUGINS_FDFAKECARDREADER_HPP_
