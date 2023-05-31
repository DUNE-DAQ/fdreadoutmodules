/**
 * @file DummyModule.cpp
 *
 * Implementations of DummyModule's functions
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DummyModule.hpp"

#include "fdreadoutmodules/dummymodule/Nljs.hpp"
#include "fdreadoutmodules/dummymoduleinfo/InfoNljs.hpp"

#include <string>

namespace dunedaq::fdreadoutmodules {

DummyModule::DummyModule(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &DummyModule::do_conf);
}

void
DummyModule::init(const data_t& /* structured args */)
{}

void
DummyModule::get_info(opmonlib::InfoCollector& ci, int /* level */)
{
  dummymoduleinfo::Info info;
  info.total_amount = m_total_amount;
  info.amount_since_last_get_info_call = m_amount_since_last_get_info_call.exchange(0);

  ci.add(info);
}

void
DummyModule::do_conf(const data_t& conf_as_json)
{
  auto conf_as_cpp = conf_as_json.get<dummymodule::Conf>();
  m_some_configured_value = conf_as_cpp.some_configured_value;
}

} // namespace dunedaq::fdreadoutmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::fdreadoutmodules::DummyModule)
