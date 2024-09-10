// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Mon Sep 9 09:21:49 2024
//  Last Modified : <240909.1448>
//
//  Description	
//
//  Notes
//
//  History
//	
/////////////////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2024  Robert Heller D/B/A Deepwoods Software
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

#ifndef __DISPLAYNETWORKHEALTH_HXX
#define __DISPLAYNETWORKHEALTH_HXX

#include "openlcb/RefreshLoop.hxx"
#include "Adafruit_HX8357.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TSC2007.h"
#include "NetworkHealthScan.hxx"

// For TSC2007
#define TSC_TS_MINX 300
#define TSC_TS_MAXX 3800
#define TSC_TS_MINY 185
#define TSC_TS_MAXY 3700


namespace DisplayNetworkHealth
{

using NetworkHealthScan::NetworkHealthScan;

class DisplayNetworkHealth : public openlcb::Polling
{
public:
    DisplayNetworkHealth(Adafruit_HX8357 *display, Adafruit_TSC2007 *touch,
                         NetworkHealthScan *healthScan);
    void poll_33hz(openlcb::WriteHelper *helper, Notifiable *done) OVERRIDE;
private:
    void refreshDisplay_();
    void redrawHeading_();
    void redrawTotals_();
    void redrawButtons_();
    void processButons_();
    class ListBox {
    public:
        ListBox(Adafruit_GFX *gfx,int16_t x, int16_t y, uint8_t textSize, 
                uint8_t items);
        void RedrawList(NetworkHealthScan::NodeDB_ConstIterator begin,
                        NetworkHealthScan::NodeDB_ConstIterator end);
        void RedrawList(uint8_t itemOffset);
        void RedrawList();
        void CheckButtons(int16_t x, int16_t y);
    private:
        Adafruit_GFX *gfx_;
        int16_t x_, y_;
        uint8_t textSize_;
        uint8_t items_;
        uint8_t itemOffset_;
        NetworkHealthScan::NodeDB_ConstIterator begin_,end_;
        Adafruit_GFX_Button prev_;
        Adafruit_GFX_Button next_;
        bool havePrev_, haveNext_;
    };
    Adafruit_HX8357 *display_;
    Adafruit_TSC2007 *touch_;
    NetworkHealthScan *healthScan_;
    size_t lastTotal_;
    ListBox nodelist_;
    Adafruit_GFX_Button rescan_;
    Adafruit_GFX_Button resetDB_;
    static constexpr const uint16_t LISTBOX_X = 0;
    static constexpr const uint16_t LISTBOX_Y = 66;
    static constexpr const uint8_t  LISTBOX_TSIZE = 3;
    static constexpr const uint8_t  LISTBOX_ITEMS = 7;
    static constexpr const int16_t  BUTTON1_X = 20;
    static constexpr const int16_t  BUTTON2_X = 260;
    static constexpr const int16_t  BUTTONS_Y = 288;
    static constexpr const uint16_t BUTTONS_W = 220;
    static constexpr const uint16_t BUTTONS_H = 32;
    static constexpr const char BUTTON1[] = "Rescan";
    static constexpr const char BUTTON2[] = "Reset";
    static constexpr const uint8_t BUTTONS_TXSZ = 4;
    static constexpr const int16_t HEADING_X = 0;
    static constexpr const int16_t HEADING_Y = 0;
    static constexpr const uint8_t HEADING_TXSZ = 8;
    static constexpr const uint16_t HEADING_COLOR=HX8357_BLUE;
    static constexpr const uint16_t HEADING_BG=HX8357_WHITE;
    static constexpr const char HEADING[] = "HEALTH";
    static constexpr const int16_t TOTALS_X = 0;
    static constexpr const int16_t TOTALS_Y = 264;
    static constexpr const uint8_t TOTALS_TXSZ = 3;
    static constexpr const uint16_t TOTALS_COLOR=HX8357_GREEN;
    static constexpr const uint16_t TOTALS_BG=HX8357_WHITE;
    static constexpr const char TOTALS_FORMAT[]="%3dT,%3dF,%2dM,%2dA";
    static constexpr const int16_t TSMin_x = TSC_TS_MINX;
    static constexpr const int16_t TSMax_x = TSC_TS_MAXX;
    static constexpr const int16_t TSMin_y = TSC_TS_MINY;
    static constexpr const int16_t TSMax_y = TSC_TS_MAXY;
};
}

#endif // __DISPLAYNETWORKHEALTH_HXX

