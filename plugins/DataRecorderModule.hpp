/**
 * @file DataRecorderModule.hpp Module to record data
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef FDREADOUTMODULES_PLUGINS_DATARECORDER_HPP_
#define FDREADOUTMODULES_PLUGINS_DATARECORDER_HPP_

#include "appfwk/DAQModule.hpp"
#include "utilities/WorkerThread.hpp"

#include "datahandlinglibs/concepts/RecorderConcept.hpp"
#include "datahandlinglibs/recorderconfig/Structs.hpp"
#include "datahandlinglibs/utils/BufferedFileWriter.hpp"

#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace dunedaq {
namespace fdreadoutmodules {

class DataRecorderModule : public dunedaq::appfwk::DAQModule
{
public:
  explicit DataRecorderModule(const std::string& name);

  DataRecorderModule(const DataRecorderModule&) = delete;
  DataRecorderModule& operator=(const DataRecorderModule&) = delete;
  DataRecorderModule(DataRecorderModule&&) = delete;
  DataRecorderModule& operator=(DataRecorderModule&&) = delete;

  void init(const CommandData_t& obj) override;
protected:
  void generate_opmon_data() override;
private:
  // Commands
  void do_conf(const CommandData_t& obj);
  void do_scrap(const CommandData_t& obj);
  void do_start(const CommandData_t& obj);
  void do_stop(const CommandData_t& obj);

  std::shared_ptr<datahandlinglibs::RecorderConcept> recorder;
};
} // namespace datahandlinglibs
} // namespace dunedaq

#endif // FDREADOUTMODULES_PLUGINS_DATARECORDER_HPP_
