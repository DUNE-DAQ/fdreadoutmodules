/**
 * @file FDFakeCardReader.hpp FarDetector FakeCardReader
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FDREADOUTMODULES_PLUGINS_FDFAKECARDREADER_HPP_
#define FDREADOUTMODULES_PLUGINS_FDFAKECARDREADER_HPP_

#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Structs.hpp"
#include "appfwk/DAQModule.hpp"

#include "readoutmodules/FakeCardReaderBase.hpp"

#include <string>

namespace dunedaq {
namespace fdreadoutmodules {

class FDFakeCardReader : public dunedaq::appfwk::DAQModule,
                         private dunedaq::readoutmodules::FakeCardReaderBase
{
public:
  using inherited_fcr = dunedaq::readoutmodules::FakeCardReaderBase;
  using inherited_mod = dunedaq::appfwk::DAQModule;
  /**
   * @brief FDFakeCardReader Constructor
   * @param name Instance name for this FDFakeCardReader instance
   */
  explicit FDFakeCardReader(const std::string& name);

  FDFakeCardReader(const FDFakeCardReader&) = delete;            ///< FDFakeCardReader is not copy-constructible
  FDFakeCardReader& operator=(const FDFakeCardReader&) = delete; ///< FDFakeCardReader is not copy-assignable
  FDFakeCardReader(FDFakeCardReader&&) = delete;                 ///< FDFakeCardReader is not move-constructible
  FDFakeCardReader& operator=(FDFakeCardReader&&) = delete;      ///< FDFakeCardReader is not move-assignable

  void init(const data_t&) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

  std::unique_ptr<readoutlibs::SourceEmulatorConcept>
  create_source_emulator(const appfwk::app::ConnectionReference qi, std::atomic<bool>& run_marker) override;

};

} // namespace fdreadoutmodules
} // namespace dunedaq

#endif // FDREADOUTMODULES_PLUGINS_FDFAKECARDREADER_HPP_
