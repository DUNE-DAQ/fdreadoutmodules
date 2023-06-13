/**
 * @file FragmentConsumer.cpp Module that consumes fragments from a queue
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef FDREADOUTMODULES_PLUGINS_FRAGMENTCONSUMER_HPP_
#define FDREADOUTMODULES_PLUGINS_FRAGMENTCONSUMER_HPP_

#include "readoutlibs/ReadoutLogging.hpp"

#include "DummyConsumer.hpp"
#include "DummyConsumer.cpp"
#include "daqdataformats/Fragment.hpp"
#include "fddetdataformats/WIBFrame.hpp"
#include "fddetdataformats/DAPHNEFrame.hpp"


#include <memory>
#include <string>

using namespace dunedaq::readoutlibs::logging;

namespace dunedaq {
namespace fdreadoutmodules {

class FragmentConsumer : public DummyConsumer<std::unique_ptr<dunedaq::daqdataformats::Fragment>>
{
public:
  explicit FragmentConsumer(const std::string name)
    : DummyConsumer<std::unique_ptr<dunedaq::daqdataformats::Fragment>>(name)
  {}

  void packet_callback(std::unique_ptr<dunedaq::daqdataformats::Fragment>& packet) override
  {
    dunedaq::daqdataformats::FragmentHeader header = packet->get_header();
    TLOG_DEBUG(TLVL_WORK_STEPS) << header;
    validate(*packet.get());
  }

  // Only does wib and daphne validation for now
  void validate(dunedaq::daqdataformats::Fragment& fragment)
  {
    return;
    if (fragment.get_size() - sizeof(daqdataformats::FragmentHeader) == 0) {
      TLOG() << "Encountered empty fragment";
      return;
    } else if ((fragment.get_header().fragment_type ==
                static_cast<daqdataformats::fragment_type_t>(daqdataformats::FragmentType::kProtoWIB)) ||
               (static_cast<fddetdataformats::WIBFrame*>(fragment.get_data())->get_wib_header()->sof == 0)) {
      int num_frames = (fragment.get_size() - sizeof(daqdataformats::FragmentHeader)) / sizeof(fddetdataformats::WIBFrame);
      auto window_begin = fragment.get_header().window_begin;
      auto window_end = fragment.get_header().window_end;

      fddetdataformats::WIBFrame* first_frame = static_cast<fddetdataformats::WIBFrame*>(fragment.get_data());
      fddetdataformats::WIBFrame* last_frame = reinterpret_cast<fddetdataformats::WIBFrame*>( // NOLINT
        static_cast<char*>(fragment.get_data()) + (num_frames - 1) * sizeof(fddetdataformats::WIBFrame));                          // NOLINT

      if (!((first_frame->get_timestamp() >= window_begin) && (first_frame->get_timestamp() < window_begin + 25))) {
        TLOG() << "First fragment not correctly aligned";
      }
      if (!((last_frame->get_timestamp() < window_end) && (last_frame->get_timestamp() >= window_end - 25))) {
        TLOG() << "Last fragment not correctly aligned";
      }

      for (int i = 0; i < num_frames; ++i) {
        fddetdataformats::WIBFrame* frame = reinterpret_cast<fddetdataformats::WIBFrame*>( // NOLINT
          static_cast<char*>(fragment.get_data()) + (i * sizeof(fddetdataformats::WIBFrame)));
        if (frame->get_timestamp() < fragment.get_header().window_begin ||
            frame->get_timestamp() >= fragment.get_header().window_end) {
          TLOG() << "Fragment validation encountered frame not fitting the requested window";
        }
      }
    } else if (fragment.get_header().fragment_type ==
               static_cast<daqdataformats::fragment_type_t>(daqdataformats::FragmentType::kDAPHNE)) {
      int num_frames = (fragment.get_size() - sizeof(daqdataformats::FragmentHeader)) / 584;

      for (int i = 0; i < num_frames; ++i) {
        fddetdataformats::DAPHNEFrame* frame = reinterpret_cast<fddetdataformats::DAPHNEFrame*>( // NOLINT
          static_cast<char*>(fragment.get_data()) + (i * 584));
        if (frame->get_timestamp() < fragment.get_header().window_begin ||
            frame->get_timestamp() >= fragment.get_header().window_end) {
          TLOG() << "Fragment validation encountered fragment not fitting the requested window";
        }
      }
    }
  }
};

} // namespace fdreadoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::fdreadoutmodules::FragmentConsumer)

#endif // FDREADOUTMODULES_PLUGINS_FRAGMENTCONSUMER_HPP_
