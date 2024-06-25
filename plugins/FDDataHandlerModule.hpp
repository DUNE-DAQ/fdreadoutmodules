/**
 * @file FDDataHandlerModule.hpp FarDetector Generic readout
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FDREADOUTMODULES_PLUGINS_FDDATALINKHANDLER_HPP_
#define FDREADOUTMODULES_PLUGINS_FDDATALINKHANDLER_HPP_

//#include "appfwk/cmd/Nljs.hpp"
//#include "appfwk/app/Nljs.hpp"
//#include "appfwk/cmd/Structs.hpp"
#include "appfwk/DAQModule.hpp"

#include "datahandlinglibs/DataLinkHandlerBase.hpp"

#include <string>

namespace dunedaq {
namespace fdreadoutmodules {

class FDDataHandlerModule : public dunedaq::appfwk::DAQModule,
                          public dunedaq::datahandlinglibs::DataLinkHandlerBase
{
public:
  using inherited_dlh = dunedaq::datahandlinglibs::DataLinkHandlerBase;
  using inherited_mod = dunedaq::appfwk::DAQModule;
  /**
   * @brief FDDataHandlerModule Constructor
   * @param name Instance name for this FDDataHandlerModule instance
   */
  explicit FDDataHandlerModule(const std::string& name);

  FDDataHandlerModule(const FDDataHandlerModule&) = delete;            ///< FDDataHandlerModule is not copy-constructible
  FDDataHandlerModule& operator=(const FDDataHandlerModule&) = delete; ///< FDDataHandlerModule is not copy-assignable
  FDDataHandlerModule(FDDataHandlerModule&&) = delete;                 ///< FDDataHandlerModule is not move-constructible
  FDDataHandlerModule& operator=(FDDataHandlerModule&&) = delete;      ///< FDDataHandlerModule is not move-assignable

  void init(std::shared_ptr<appfwk::ModuleConfiguration> cfg) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

  std::unique_ptr<datahandlinglibs::DataHandlingConcept>
  create_readout(const appmodel::DataHandlerModule* modconf, std::atomic<bool>& run_marker) override;

};

} // namespace fdreadoutmodules
} // namespace dunedaq

#endif // FDREADOUTMODULES_PLUGINS_FDDATALINKHANDLER_HPP_
