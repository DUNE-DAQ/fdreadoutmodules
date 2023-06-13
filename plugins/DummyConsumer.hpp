/**
 * @file DummyConsumer.hpp Module that consumes data from a queue and does nothing with it
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef FDREADOUTMODULES_PLUGINS_DUMMYCONSUMER_HPP_
#define FDREADOUTMODULES_PLUGINS_DUMMYCONSUMER_HPP_

#include "appfwk/DAQModule.hpp"
#include "iomanager/IOManager.hpp"
#include "dfmessages/TimeSync.hpp"

#include "readoutlibs/utils/ReusableThread.hpp"

#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace dunedaq {
namespace fdreadoutmodules {

template<class T>
class DummyConsumer : public dunedaq::appfwk::DAQModule
{
public:
  explicit DummyConsumer(const std::string& name);

  DummyConsumer(const DummyConsumer&) = delete;
  DummyConsumer& operator=(const DummyConsumer&) = delete;
  DummyConsumer(DummyConsumer&&) = delete;
  DummyConsumer& operator=(DummyConsumer&&) = delete;

  void init(const nlohmann::json& obj) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

protected:
  virtual void packet_callback(T& /*packet*/) {}

private:
  // Commands
  void do_start(const nlohmann::json& obj);
  void do_stop(const nlohmann::json& obj);
  void do_work();

  // Queue
  using source_t = dunedaq::iomanager::ReceiverConcept<T>;
  std::shared_ptr<source_t> m_data_receiver;

  // Threading
  readoutlibs::ReusableThread m_work_thread;
  std::atomic<bool> m_run_marker;

  // Stats
  std::atomic<int> m_packets_processed{ 0 };
};

} // namespace fdreadoutmodules
} // namespace dunedaq

#endif // FDREADOUTMODULES_PLUGINS_DUMMYCONSUMER_HPP_
