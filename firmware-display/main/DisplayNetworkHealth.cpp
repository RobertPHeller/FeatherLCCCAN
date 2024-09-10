// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Mon Sep 9 09:39:50 2024
//  Last Modified : <240909.1520>
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

static const char rcsid[] = "@(#) : $Id$";

#include "openlcb/RefreshLoop.hxx"
#include "Adafruit_HX8357.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TSC2007.h"
#include "hardware.hxx"
#include "DisplayNetworkHealth.hxx"
#include "NetworkHealthScan.hxx"
#include "StringUtils.hxx"

namespace Arduino
{
static long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
}

namespace DisplayNetworkHealth
{

DisplayNetworkHealth::DisplayNetworkHealth(Adafruit_HX8357 *display, 
                                           Adafruit_TSC2007 *touch,
                                           NetworkHealthScan *healthScan)
      : display_(display)
, touch_(touch)
, healthScan_(healthScan)
, lastTotal_(0)
, nodelist_(display,LISTBOX_X,LISTBOX_Y,LISTBOX_TSIZE,LISTBOX_ITEMS)
{
    rescan_.initButtonUL(display_,BUTTON1_X,BUTTONS_Y,BUTTONS_W,BUTTONS_H,
                         HX8357_WHITE,HX8357_BLACK,HX8357_MAGENTA,
                         BUTTON1,BUTTONS_TXSZ);
    resetDB_.initButtonUL(display_,BUTTON2_X,BUTTONS_Y,BUTTONS_W,BUTTONS_H,
                          HX8357_WHITE,HX8357_BLACK,HX8357_MAGENTA,
                          BUTTON2,BUTTONS_TXSZ);
    refreshDisplay_();
}

void DisplayNetworkHealth::poll_33hz(openlcb::WriteHelper *helper, 
                                     Notifiable *done)
{
    if (healthScan_->Total() != lastTotal_)
    {
        nodelist_.RedrawList(healthScan_->NodeDB_Begin(),
                             healthScan_->NodeDB_End());
        redrawTotals_();
    }
    processButons_();
    done->notify();
}

void DisplayNetworkHealth::refreshDisplay_()
{
    display_->fillScreen(HX8357_BLACK);
    redrawHeading_();
    nodelist_.RedrawList(healthScan_->NodeDB_Begin(),
                         healthScan_->NodeDB_End());
    redrawTotals_();
    redrawButtons_();
}

void DisplayNetworkHealth::redrawHeading_()
{
    display_->setCursor(HEADING_X,HEADING_Y);
    display_->setTextSize(HEADING_TXSZ);
    display_->setTextColor(HEADING_COLOR,HEADING_BG);
    display_->print(HEADING);
}

void DisplayNetworkHealth::redrawTotals_()
{
    char buffer[20];
    display_->setCursor(TOTALS_X,TOTALS_Y);
    display_->setTextSize(TOTALS_TXSZ);
    display_->setTextColor(TOTALS_COLOR,TOTALS_BG);
    snprintf(buffer,sizeof(buffer),TOTALS_FORMAT,
             healthScan_->Total(),healthScan_->Found(),
             healthScan_->Missing(),healthScan_->Added());
    display_->print(buffer);
}

void DisplayNetworkHealth::redrawButtons_()
{
    rescan_.drawButton();
    resetDB_.drawButton();
}


void DisplayNetworkHealth::processButons_()
{
    TS_Point p = touch_->getPoint();
    if (((p.x == 0) && (p.y == 0)) || (p.z < 10))
    {
        // this is our way of tracking touch 'release'!
        p.x = p.y = p.z = -1;
    }
    // Scale from ~0->4000 to  tft.width using the calibration #'s
    if (p.z != -1)
    {
        int py = Arduino::map(p.x, TSMax_x, TSMin_x, 0, display_->height());
        int px = Arduino::map(p.y, TSMin_y, TSMax_y, 0, display_->width());
        p.x = px;
        p.y = py;
    }
    if (rescan_.contains(p.x, p.y))
    {
        rescan_.press(true);
        
    }
    else
    {
        rescan_.press(false);
    }
    if (resetDB_.contains(p.x, p.y))
    {
        resetDB_.press(true);
    }
    else
    {
        resetDB_.press(false);
    }
    if (rescan_.justReleased())
    {
        rescan_.drawButton(false);
        healthScan_->ScanNetwork();
    }
    if (rescan_.justPressed())
    {
        rescan_.drawButton(true);
        return;
    }
    if (resetDB_.justReleased())
    {
        resetDB_.drawButton(false);
        healthScan_->ResetNodeDB();
    }
    if (resetDB_.justPressed())
    {
        resetDB_.drawButton(true);
        return;
    }
    nodelist_.CheckButtons(p.x, p.y);
}

DisplayNetworkHealth::ListBox::ListBox(Adafruit_GFX *gfx,int16_t x, int16_t y, 
                                       uint8_t textSize, uint8_t items)
      : gfx_(gfx)
, x_(x)
, y_(y)
, textSize_(textSize)
, items_(items)
, itemOffset_(0)
, havePrev_(false)
, haveNext_(false)
{
    uint16_t w_2 = gfx_->width()/2;
    prev_.initButtonUL(gfx_,x_,y_+(items_*(textSize_*8)),w_2,(textSize_*8),
                       HX8357_WHITE,HX8357_BLACK,HX8357_MAGENTA,
                       "Prev",textSize_);
    next_.initButtonUL(gfx_,x_+w_2,y_+(items_*(textSize_*8)),w_2,
                       (textSize_*8),HX8357_WHITE,HX8357_BLACK,HX8357_MAGENTA,
                       "Next",textSize_);
}
void DisplayNetworkHealth::ListBox::RedrawList(
                              NetworkHealthScan::NodeDB_ConstIterator begin,
                              NetworkHealthScan::NodeDB_ConstIterator end)
{
    begin_ = begin;
    end_ = end;
    RedrawList(0);
}

void DisplayNetworkHealth::ListBox::RedrawList(uint8_t itemOffset)
{
    itemOffset_ = itemOffset;
    RedrawList();
}

void DisplayNetworkHealth::ListBox::RedrawList()
{
    gfx_->fillRect(x_,y_,gfx_->width(),(textSize_*8)*(items_+1),HX8357_BLACK);
    int16_t y=y_;
    gfx_->setTextSize(textSize_);
    gfx_->setTextColor(HX8357_CYAN,HX8357_BLACK);
    auto it = begin_;
    std::advance(it, itemOffset_);
    havePrev_ = itemOffset_ > 0;
    haveNext_ = std::distance(it,end_) > items_;
    for (uint8_t i = 0; i < items_; i++)
    {
        if (it == end_) break;
        string nodeidstring = utils::node_id_to_string(it->second.node_id);
        gfx_->setCursor(x_,y);
        gfx_->print(nodeidstring.c_str());
        y += textSize_*8;
        it++;
    }
    if (havePrev_) prev_.drawButton();
    if (haveNext_) next_.drawButton();
}

void DisplayNetworkHealth::ListBox::CheckButtons(int16_t x, int16_t y)
{
    if (havePrev_)
    {
        if (prev_.contains(x,y))
        {
            prev_.press(true);
        }
        else
        {
            prev_.press(false);
        }
        if (prev_.justReleased())
        {
            prev_.drawButton(false);
            if (itemOffset_ >= items_) 
            {
                RedrawList(itemOffset_-items_);
            }
        }
        if (prev_.justPressed())
        {
            prev_.drawButton(true);
            return;
        }
    }
    if (haveNext_)
    {
        if (next_.contains(x,y))
        {
            next_.press(true);
        }
        else
        {
            next_.press(false);
        }
        if (next_.justReleased())
        {
            next_.drawButton(false);
            if (std::distance(begin_,end_) > itemOffset_+items_) 
            {
                RedrawList(itemOffset_+items_);
            }
        }
        if (next_.justPressed())
        {
            next_.drawButton(true);
            return;
        }
    }
}

}
