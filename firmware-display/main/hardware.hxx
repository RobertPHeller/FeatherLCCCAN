// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Sun Jan 15 16:13:22 2023
//  Last Modified : <240908.2026>
//
//  Description	
//
//  Notes
//
//  History
//	
/////////////////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2023  Robert Heller D/B/A Deepwoods Software
//			51 Locke Hill Road
//			Wendell, MA 01379-9728
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// 
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __HARDWARE_HXX
#define __HARDWARE_HXX

#include <freertos_drivers/arduino/DummyGPIO.hxx>
#include <freertos_drivers/esp32/Esp32Gpio.hxx>
#include <os/Gpio.hxx>
#include <utils/GpioInitializer.hxx>

/// Display Data/Command
GPIO_PIN(DRs, GpioOutputSafeHighInvert, 10);
/// TouchScreen data available
GPIO_PIN(TSC_IRQ,GpioInputNP,6);

/// SPI pins
static constexpr gpio_num_t CONFIG_MOSI = GPIO_NUM_35;
static constexpr gpio_num_t CONFIG_MISO = GPIO_NUM_37;
static constexpr gpio_num_t CONFIG_SCLK = GPIO_NUM_36;

/// SPI Chip Selects
static constexpr gpio_num_t CONFIG_CardCS = GPIO_NUM_5;
static constexpr gpio_num_t CONFIG_DCS = GPIO_NUM_9;

/// GPIO Pin connected to the TWAI (CAN) Transceiver RX pin.
static constexpr gpio_num_t CONFIG_TWAI_RX_PIN = GPIO_NUM_12;

/// GPIO Pin connected to the TWAI (CAN) Transceiver TX pin.
static constexpr gpio_num_t CONFIG_TWAI_TX_PIN = GPIO_NUM_11;

// Create an initializer that can initialize all the GPIO pins in one shot
typedef GpioInitializer<DRs_Pin, TSC_IRQ_Pin> GpioInit;

#endif // __HARDWARE_HXX

