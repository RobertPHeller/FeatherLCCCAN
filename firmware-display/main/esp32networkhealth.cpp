// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Tue Sep 3 22:18:49 2024
//  Last Modified : <250314.1708>
//
//  Description	
//
//  Notes
//
//  History
//	
/////////////////////////////////////////////////////////////////////////////
/// @copyright
///    Copyright (C) 2024  Robert Heller D/B/A Deepwoods Software
///			51 Locke Hill Road
///			Wendell, MA 01379-9728
///
///    This program is free software; you can redistribute it and/or modify
///    it under the terms of the GNU General Public License as published by
///    the Free Software Foundation; either version 2 of the License, or
///    (at your option) any later version.
///
///    This program is distributed in the hope that it will be useful,
///    but WITHOUT ANY WARRANTY; without even the implied warranty of
///    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///    GNU General Public License for more details.
///
///    You should have received a copy of the GNU General Public License
///    along with this program; if not, write to the Free Software
///    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
/// @file esp32networkhealth.cpp
/// @author Robert Heller
/// @date Tue Sep 3 22:18:49 2024
/// 
///
//////////////////////////////////////////////////////////////////////////////

static const char rcsid[] = "@(#) : $Id$";


#include "sdkconfig.h"
#include "cdi.hxx"
#include "cdidata.hxx"
#include "FactoryResetHelper.hxx"
#include "HealthMonitor.hxx"
#include "fs.hxx"
#include "hardware.hxx"
#include "NvsManager.hxx"
#include "NodeRebootHelper.hxx"
#include "BootPauseHelper.hxx"

#include <algorithm>
#include <driver/i2c.h>
#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#include <freertos_includes.h>   
#include <openlcb/SimpleStack.hxx>
#include <freertos_drivers/esp32/Esp32HardwareTwai.hxx>
#include <freertos_drivers/esp32/Esp32WiFiManager.hxx>
#include <freertos_drivers/esp32/Esp32BootloaderHal.hxx>
#include <freertos_drivers/esp32/Esp32SocInfo.hxx>
#include <freertos_drivers/esp32/Esp32Ledc.hxx>
#include <openlcb/MemoryConfigClient.hxx>
#include <openlcb/RefreshLoop.hxx>
#include <openlcb/SimpleStack.hxx>
#include <utils/constants.hxx>
#include <utils/format_utils.hxx>

#include "Esp32SPI.hxx"
#include "Adafruit_HX8357.h"
#include "freertos_drivers/esp32/Esp32HardwareI2C.hxx"
#include "Adafruit_TSC2007.h"

#include "NetworkHealthScan.hxx"
#include "DisplayNetworkHealth.hxx"

///////////////////////////////////////////////////////////////////////////////
// Increase the CAN RX frame buffer size to reduce overruns when there is high
// traffic load (ie: large datagram transport).
///////////////////////////////////////////////////////////////////////////////
OVERRIDE_CONST(can_rx_buffer_size, 64);
OVERRIDE_CONST(num_memory_spaces, 8);

esp32networkhealth::ConfigDef cfg(0);
Esp32HardwareTwai twai(CONFIG_TWAI_RX_PIN, CONFIG_TWAI_TX_PIN);
Esp32SPI spibus;
Esp32SPI::SpiDevice *sdcard;
Adafruit_HX8357 display(CONFIG_DCS,DRs_Pin::instance(),nullptr);
Esp32HardwareI2C i2c("/dev/i2c");
Adafruit_TSC2007 touchscreen;

namespace openlcb
{

/// Name of CDI.xml to generate dynamically.
const char CDI_FILENAME[] = "/fs/cdi.xml";

// Path to where OpenMRN should persist general configuration data.
extern const char *const CONFIG_FILENAME = "/fs/config";

// The size of the memory space to export over the above device.
extern const size_t CONFIG_FILE_SIZE = cfg.seg().size() + cfg.seg().offset();

// Default to store the dynamic SNIP data is stored in the same persistant
// data file as general configuration data.
extern const char *const SNIP_DYNAMIC_FILENAME = "/fs/config";

/// Defines the identification information for the node. The arguments are:
///
/// - 4 (version info, always 4 by the standard
/// - Manufacturer name
/// - Model name
/// - Hardware version
/// - Software version
///
/// This data will be used for all purposes of the identification:
///
/// - the generated cdi.xml will include this data
/// - the Simple Node Ident Info Protocol will return this data
/// - the ACDI memory space will contain this data.
extern const SimpleNodeStaticValues SNIP_STATIC_DATA =
{
    4,
    SNIP_PROJECT_PAGE,
    SNIP_PROJECT_NAME,
    SNIP_HW_VERSION,
    SNIP_SW_VERSION
};

/// Modify this value every time the EEPROM needs to be cleared on the node
/// after an update.
static constexpr uint16_t CANONICAL_VERSION = CDI_VERSION;

} // namespace openlcb


extern "C"
{

void *node_reboot(void *arg)
{
    Singleton<reboothelpers::NodeRebootHelper>::instance()->reboot();
    return nullptr;
}

void reboot()
{
    os_thread_create(nullptr, nullptr, uxTaskPriorityGet(NULL) + 1, 2048
                     , node_reboot, nullptr);
}
ssize_t os_get_free_heap()
{
    return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}


namespace esp32networkhealth {


ConfigUpdateListener::UpdateAction FactoryResetHelper::apply_configuration(
                                                                           int fd, bool initial_load, BarrierNotifiable *done)
{
    // nothing to do here as we do not load config
    AutoNotify n(done);
    LOG(VERBOSE, "[CFG] apply_configuration(%d, %d)", fd, initial_load);
    if (!initial_load &&
        Singleton<nvsmanager::NvsManager>::instance()->CheckPersist())
    {
        LOG(WARNING, "[CFG] NVS has been updated requesting a restart.");
        return ConfigUpdateListener::UpdateAction::REBOOT_NEEDED;
    }
    
    return ConfigUpdateListener::UpdateAction::UPDATED;
}

void FactoryResetHelper::factory_reset(int fd)
{
    LOG(VERBOSE, "[CFG] factory_reset(%d)", fd);
    // set the name of the node to the SNIP model name
    cfg.userinfo().name().write(fd, openlcb::SNIP_STATIC_DATA.model_name);
    cfg.userinfo().description().write(fd, "");
    
}

}

void app_main()
{
    // capture the reason for the CPU reset
    uint8_t reset_reason = Esp32SocInfo::print_soc_info();
    
    LOG(INFO, "[SNIP] version:%d, manufacturer:%s, model:%s, hw-v:%s, sw-v:%s"
        , openlcb::SNIP_STATIC_DATA.version
        , openlcb::SNIP_STATIC_DATA.manufacturer_name
        , openlcb::SNIP_STATIC_DATA.model_name
        , openlcb::SNIP_STATIC_DATA.hardware_version
        , openlcb::SNIP_STATIC_DATA.software_version);
    bool reset_events = false;
    bool cleanup_config_tree = false;
    
    GpioInit::hw_init();
    spibus.hw_initbus(CONFIG_MOSI,CONFIG_MISO,CONFIG_SCLK);
    display.begin(&spibus);
    i2c.hw_init(CONFIG_SDA_PIN,CONFIG_SDL_PIN, 100000);
    touchscreen.begin();
    sdcard = spibus.mount_sd_card("/sdcard",CONFIG_CardCS);
    
    nvsmanager::NvsManager nvs;
    nvs.init(reset_reason);
    
    LOG(INFO, "[BootPauseHelper] starting...");
    
    nvsmanager::BootPauseHelper pause;
    
    pause.CheckPause();
    LOG(INFO, "[BootPauseHelper] returned...");
    
    // Check for and reset factory reset flag.
    if (nvs.should_reset_config())
    {
        cleanup_config_tree = true;
        nvs.clear_factory_reset();
    }
    
    if (nvs.should_reset_events())
    {
        reset_events = true;
        // reset the flag so we start in normal operating mode next time.
        nvs.clear_reset_events();
    }
    nvs.CheckPersist();
    
    nvs.DisplayNvsConfiguration();
    mount_fs(cleanup_config_tree);
    openlcb::SimpleCanStack stack(nvs.node_id());
    LOG(INFO, "[MAIN] SimpleCanStack allocated");
#if CONFIG_OLCB_PRINT_ALL_PACKETS
    stack.print_all_packets();
#endif
    nvs.register_virtual_memory_spaces(&stack);
    openlcb::MemoryConfigClient memory_client(stack.node(), stack.memory_config_handler());
    LOG(INFO, "[MAIN] MemoryConfigClient allocated");
#ifdef CONFIG_ESP32_WIFI_ENABLED
    openmrn_arduino::Esp32WiFiManager wifi_manager(
                                                   nvs.station_ssid(), 
                                                   nvs.station_pass(),
                                                   &stack, 
                                                   cfg.seg().olbcwifi(), 
                                                   nvs.wifi_mode(),
                                                   (uint8_t)CONFIG_OLCB_WIFI_MODE, /* uplink / hub mode */
                                                   nvs.hostname_prefix());
#endif
    esp32networkhealth::FactoryResetHelper factory_reset_helper;
    LOG(INFO, "[MAIN] FactoryResetHelper allocated");
    healthmonitor::HealthMonitor health_mon(stack.service());
    LOG(INFO, "[MAIN] HealthMonitor allocated");
    NetworkHealthScan::NetworkHealthScan 
          healthScan(stack.node(),
                     stack.service(),
                     stack.executor()->active_timers(),
                     cfg.seg().scanConfig());
    openlcb::MemorySpace *space = healthScan.NodeDBSpace();
    stack.memory_config_handler()->registry()->insert(stack.node(),
                                                      CONFIG_NodeDB_SPACE,
                                                      space);
    DisplayNetworkHealth::DisplayNetworkHealth 
          displayNetHealth(&display,&touchscreen,&healthScan);
    openlcb::RefreshLoop loopdisplay(stack.node(),{&displayNetHealth});
    LOG(INFO, "[MAIN] config file size is %d",openlcb::CONFIG_FILE_SIZE);
    // Create config file and initiate factory reset if it doesn't exist or is
    // otherwise corrupted.
    int config_fd =
          stack.create_config_file_if_needed(cfg.seg().internal_config(),
                                             openlcb::CANONICAL_VERSION,
                                             openlcb::CONFIG_FILE_SIZE);
    stack.check_version_and_factory_reset(cfg.seg().internal_config(),
                                          CDI_VERSION,
                                          cleanup_config_tree);
    reboothelpers::NodeRebootHelper node_reboot_helper(&stack, config_fd);
    
    if (reset_events)
    {
        LOG(WARNING, "[CDI] Resetting event IDs");
        stack.factory_reset_all_events(cfg.seg().internal_config(), 
                                       nvs.node_id(), config_fd);
        fsync(config_fd);
    }
    
    
    // Initialize the TWAI driver.
    twai.hw_init();
    
    // Add the TWAI port to the stack.
    stack.add_can_port_select("/dev/twai/twai0");
    healthScan.ScanNetwork();
    // Start the stack in the background using it's own task.
    stack.loop_executor();
    // At this point the OpenMRN stack is running in it's own task and we can
    // safely exit from this one. We do not need to cleanup as that will be
    // handled automatically by ESP-IDF.
}



} // extern "C"

