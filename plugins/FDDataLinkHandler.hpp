/**
 * @file FDDataLinkHandler.hpp FarDetector Generic readout
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FDREADOUTMODULES_PLUGINS_FDDATALINKHANDLER_HPP_
#define FDREADOUTMODULES_PLUGINS_FDDATALINKHANDLER_HPP_

#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Structs.hpp"
#include "appfwk/DAQModule.hpp"

#include "readoutmodules/DataLinkHandlerBase.hpp"

#include <string>

namespace dunedaq {
namespace fdreadoutmodules {

class FDDataLinkHandler : public dunedaq::appfwk::DAQModule,
                          public dunedaq::readoutmodules::DataLinkHandlerBase
{
public:
  using inherited_dlh = dunedaq::readoutmodules::DataLinkHandlerBase;
  using inherited_mod = dunedaq::appfwk::DAQModule;
  /**
   * @brief FDDataLinkHandler Constructor
   * @param name Instance name for this FDDataLinkHandler instance
   */
  explicit FDDataLinkHandler(const std::string& name);

  FDDataLinkHandler(const FDDataLinkHandler&) = delete;            ///< FDDataLinkHandler is not copy-constructible
  FDDataLinkHandler& operator=(const FDDataLinkHandler&) = delete; ///< FDDataLinkHandler is not copy-assignable
  FDDataLinkHandler(FDDataLinkHandler&&) = delete;                 ///< FDDataLinkHandler is not move-constructible
  FDDataLinkHandler& operator=(FDDataLinkHandler&&) = delete;      ///< FDDataLinkHandler is not move-assignable

  void init(const data_t& args) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

  std::unique_ptr<readoutlibs::ReadoutConcept>
  create_readout(const nlohmann::json& args, std::atomic<bool>& run_marker) override;

};

} // namespace fdreadoutmodules
} // namespace dunedaq

#endif // FDREADOUTMODULES_PLUGINS_FDDATALINKHANDLER_HPP_
