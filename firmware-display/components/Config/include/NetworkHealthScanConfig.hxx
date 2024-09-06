// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Wed Sep 4 13:28:32 2024
//  Last Modified : <240904.1334>
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

#ifndef __NETWORKHEALTHSCANCONFIG_HXX
#define __NETWORKHEALTHSCANCONFIG_HXX

#include "openlcb/ConfigRepresentation.hxx"


namespace NetworkHealthScan
{

CDI_GROUP(NetworkHealthScanConfig);
CDI_GROUP_ENTRY(scan, openlcb::EventConfigEntry,
                Name("(C) Event to start a network scan"))
CDI_GROUP_ENTRY(resetList, openlcb::EventConfigEntry,
                Name("(C) Event to reset the node list and rescan"));
CDI_GROUP_ENTRY(scanOK, openlcb::EventConfigEntry,
                Name("(P) Send this event when scan completes with no errors"));
CDI_GROUP_ENTRY(scanMissing, openlcb::EventConfigEntry,
                Name("(P) Send this event when scan completes with nodes missing"));
CDI_GROUP_ENTRY(scanAdded, openlcb::EventConfigEntry,
                Name("(P) Send this event when scan completes with nodes added"));
CDI_GROUP_END();

}

#endif // __NETWORKHEALTHSCANCONFIG_HXX

