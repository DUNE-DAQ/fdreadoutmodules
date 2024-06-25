/**
 * @file DummyConsumer.cpp DummyConsumer implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "DummyConsumer.hpp"

#include "appfwk/DAQModuleHelper.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "logging/Logging.hpp"

#include "datahandlinglibs/DataHandlingIssues.hpp"
#include "datahandlinglibs/ReadoutLogging.hpp"

#include "fdreadoutmodules/dummyconsumerinfo/InfoNljs.hpp"

#include <string>

namespace dunedaq {
namespace fdreadoutmodules {

using namespace logging;

template<class T>
DummyConsumer<T>::DummyConsumer(const std::string& name)
  : DAQModule(name)
  , m_work_thread(0)
{
  register_command("start", &DummyConsumer::do_start);
  register_command("stop_trigger_sources", &DummyConsumer::do_stop);
}

template<class T>
void
DummyConsumer<T>::init(const data_t& args)
{
  try {
    auto qi = appfwk::connection_index(args, { "input_queue" });
    m_data_receiver = get_iom_receiver<T>(qi["input_queue"]);
  } catch (const ers::Issue& excpt) {
    throw datahandlinglibs::GenericResourceQueueError(ERS_HERE, "input_queue", get_name(), excpt);
  }
}

template<class T>
void
DummyConsumer<T>::get_info(opmonlib::InfoCollector& ci, int /* level */)
{
  dummyconsumerinfo::Info info;
  info.packets_processed = m_packets_processed;

  ci.add(info);
}

template<class T>
void
DummyConsumer<T>::do_start(const data_t& /* args */)
{
  m_packets_processed = 0;
  m_run_marker.store(true);
  m_work_thread.set_work(&DummyConsumer::do_work, this);
}

template<class T>
void
DummyConsumer<T>::do_stop(const data_t& /* args */)
{
  m_run_marker.store(false);
  while (!m_work_thread.get_readiness()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

template<class T>
void
DummyConsumer<T>::do_work()
{
  T element;
  while (m_run_marker) {
    try {
      element = m_data_receiver->receive(std::chrono::milliseconds(100));
      packet_callback(element);
      m_packets_processed++;
    } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
      continue;
    }
  }
}

} // namespace fdreadoutmodules
} // namespace dunedaq
