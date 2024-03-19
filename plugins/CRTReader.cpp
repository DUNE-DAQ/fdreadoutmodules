/**
 * @file CRTReader.cpp CRTReader class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CRTReader.hpp"
#include "CommonIssues.hpp"

#include "appfwk/DAQModuleHelper.hpp"
#include "iomanager/IOManager.hpp"
#include "logging/Logging.hpp"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "CRTReader" // NOLINT
#define TLVL_ENTER_EXIT_METHODS 10
#define TLVL_CRTREADER 15

namespace dunedaq {
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::CRTTypeAdapter, "CRTFrame")
namespace fdreadoutmodules{

CRTReader::CRTReader(const std::string& name)
  : DAQModule(name)
  , thread_(std::bind(&CRTReader::do_work, this, std::placeholders::_1))
  , outputQueue_(nullptr)
  , queueTimeout_(100)
{
  register_command("start", &CRTReader::do_start);
  register_command("stop", &CRTReader::do_stop);
}

void
CRTReader::init(const nlohmann::json& iniobj)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  auto qi = appfwk::connection_index(iniobj, {  "output_100" });
  try {
    outputQueue_ = get_iom_sender<fdreadoutlibs::types::CRTTypeAdapter>(qi["output_100"]);
  } catch (const ers::Issue& excpt) {
    throw InvalidQueueFatalError(ERS_HERE, get_name(), "output", excpt);
  }
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

void
CRTReader::do_start(const nlohmann::json& /*startobj*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_start() method";
  thread_.start_working_thread();
  
  TLOG() << get_name() << " successfully started";
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_start() method";
}

void
CRTReader::do_stop(const nlohmann::json& /*stopobj*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_stop() method";
  thread_.stop_working_thread();

  hardware_interface_->StopDatataking();
  hardware_interface_->FreeReadoutBuffer(readout_buffer_);

  TLOG() << get_name() << " successfully stopped";
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_stop() method";
}

void
CRTReader::do_work(std::atomic<bool>& running_flag)
{
  hardware_interface_ = new CRTInterface("/nfs/home/madmurph/VT_daq/ICARUS_DAQ/readout/DataFolder",3);
  
  hardware_interface_->AllocateReadoutBuffer(&readout_buffer_);
  hardware_interface_->StartDatataking();
  hardware_interface_->SetBaselines();
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_work() method";
  int sentCount = 0;
  while (running_flag.load()) {
      size_t bytes_read = 0;
      hardware_interface_->FillBuffer(readout_buffer_, &bytes_read);
      if(bytes_read>0){
        /*const uint64_t daqheader1 = *(uint64_t*)(readout_buffer_);
        const uint64_t daqheader2 = *(uint64_t*)(readout_buffer_+8);

        uint64_t version = (daqheader1 >> 58) & 0x000000000000003F; //first 6 bits
        uint64_t det_id = (daqheader1 >> 52) & 0x000000000000003F;  //next 6 bits
        uint64_t crate_id = (daqheader1 >> 42) & 0x00000000000003FF;//next 10 bits
        uint64_t slot_id = (daqheader1 >> 38) & 0x000000000000000F;//next 4 bits
        uint64_t stream_id = (daqheader1 >> 30) & 0x00000000000000FF;//next 8 bits

        printf("version: %lu\tdet_id: %lu\tcrate_id: %lu\tslot_id: %lu\tstream_id: %lu\n",version, det_id, crate_id, slot_id, stream_id);

        printf("daqheader1: %lu\tdaqheader2: %lu\n",daqheader1, daqheader2);

        const char magic_number = *(char*)(readout_buffer_+16);
        const uint8_t hit_count = *(uint8_t*)(readout_buffer_+1+16);
        const uint16_t module_id = *(uint16_t*)(readout_buffer_ +2+16);
        const uint32_t upper_global = *(uint32_t*)(readout_buffer_ + 4+16);
        const uint32_t lower_global = *(uint32_t*)(readout_buffer_ + 8+16);
        const uint32_t mhz_counter = *(uint32_t*)(readout_buffer_ + 12+16);

        printf("Magic: %c\tCount: %u\tModule: %u\n",magic_number, hit_count, module_id);
        printf("Zero?: %u\tZero again?: %u\n",upper_global, lower_global);
        printf("MHz counter: %u\n",mhz_counter);

        char hit_magic = *(char*)(readout_buffer_ + 16+16);
        size_t j = 0;
        while(16 + j*4 + 4 <= bytes_read){
          std::cout << "j=" << j << std::endl;
          hit_magic = *(char*)(readout_buffer_ + 16 + j*4+16);
          const uint8_t hit_channel = *(uint8_t*)(readout_buffer_ + 16 + j*4 + 1+16);
          const int16_t hit_adc = *(int16_t*)(readout_buffer_ + 16 + j*4 + 2+16);
          printf("Magic: %c\tChannel: %u\tADC: %i\n",hit_magic,hit_channel,hit_adc);
          //if(j+1 > hit_count){
          //  std::cout << "More hits than there should be?\n";
          //}
          j++;
        }*/
	fdreadoutlibs::types::CRTTypeAdapter to_send;
	//std::cout << "Before memcpy: get_timestamp = " << to_send.get_first_timestamp() << std::endl;
	//std::memcpy(&(to_send.data[0]), &readout_buffer_,288);	
	for(int k=0;k<288;k++){
	  to_send.data[k] = *(char*)(readout_buffer_+k);
	}
	std::cout << "Test: get_timestamp = " << to_send.get_first_timestamp() << std::endl;
	bool successfullyWasSent = false;
        while (!successfullyWasSent && running_flag.load()) {
          TLOG_DEBUG(TLVL_CRTREADER) << get_name() << ": Pushing the reversed list onto the output queue";
          try {
          
          outputQueue_->send(std::move(to_send), queueTimeout_);
          successfullyWasSent = true;
	  //std::cout << "Successfully sent\n";
          ++sentCount;
        } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
          std::ostringstream oss_warn;
          oss_warn << "push to output queue \"" << outputQueue_->get_name() << "\"";
          ers::warning(dunedaq::iomanager::TimeoutExpired(
            ERS_HERE,
            get_name(),
            oss_warn.str(),
            std::chrono::duration_cast<std::chrono::milliseconds>(queueTimeout_).count()));
          }
        } 
      
      }
      else{
        continue;
      }
  }
  /*int receivedCount = 0;
  int sentCount = 0;
  std::vector<int> workingVector;

  while (running_flag.load()) {
    TLOG_DEBUG(TLVL_CRTREADER) << get_name() << ": Going to receive data from input queue";
    try {
      workingVector = inputQueue_->receive(queueTimeout_).list;
    } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
      // it is perfectly reasonable that there might be no data in the queue
      // some fraction of the times that we check, so we just continue on and try again
      continue;
    }

    ++receivedCount;
    TLOG_DEBUG(TLVL_CRTREADER) << get_name() << ": Received list #" << receivedCount << ". It has size "
                                   << workingVector.size() << ". Reversing its contents";
    std::reverse(workingVector.begin(), workingVector.end());

    std::ostringstream oss_prog;
    oss_prog << "Reversed list #" << receivedCount << ", new contents " << workingVector << " and size "
             << workingVector.size() << ". ";
    ers::debug(ProgressUpdate(ERS_HERE, get_name(), oss_prog.str()));

    bool successfullyWasSent = false;
    while (!successfullyWasSent && running_flag.load()) {
      TLOG_DEBUG(TLVL_CRTREADER) << get_name() << ": Pushing the reversed list onto the output queue";
      try {
        IntList wrapped(workingVector);
        outputQueue_->send(std::move(wrapped), queueTimeout_);
        successfullyWasSent = true;
        ++sentCount;
      } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
        std::ostringstream oss_warn;
        oss_warn << "push to output queue \"" << outputQueue_->get_name() << "\"";
        ers::warning(dunedaq::iomanager::TimeoutExpired(
          ERS_HERE,
          get_name(),
          oss_warn.str(),
          std::chrono::duration_cast<std::chrono::milliseconds>(queueTimeout_).count()));
      }
    }
    TLOG_DEBUG(TLVL_CRTREADER) << get_name() << ": End of do_work loop";
  }

  std::ostringstream oss_summ;
  oss_summ << ": Exiting do_work() method, received " << receivedCount << " lists and successfully sent " << sentCount
           << ". ";
  ers::info(ProgressUpdate(ERS_HERE, get_name(), oss_summ.str()));
  */
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_work() method";
}

} // namespace fdreadoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::fdreadoutmodules::CRTReader)

// Local Variables:
// c-basic-offset: 2
// End:
