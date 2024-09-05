// -!- C++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Wed Sep 4 14:31:24 2024
//  Last Modified : <240904.2014>
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

#include "openlcb/EventHandlerTemplates.hxx"
#include "openlcb/ConfigRepresentation.hxx"
#include "utils/ConfigUpdateListener.hxx"
#include "utils/ConfigUpdateService.hxx"
#include "openlcb/RefreshLoop.hxx"
#include "openlcb/SimpleStack.hxx"
#include "executor/Timer.hxx"
#include "executor/Notifiable.hxx"
#include "openlcb/NodeBrowser.hxx"
#include "openlcb/SNIPClient.hxx"
#include <stdio.h>
#include <stdlib.h>
#include "utils/logging.h"
#include <string>
#include "NetworkHealthScanConfig.hxx"
#include "NetworkHealthScan.hxx"

namespace NetworkHealthScan
{

void NetworkHealthScan::NetworkHealthConsumer::handle_identify_global(const openlcb::EventRegistryEntry &registry_entry, 
                                                                      openlcb::EventReport *event, 
                                                                      BarrierNotifiable *done)
{
}

void NetworkHealthScan::NetworkHealthConsumer::handle_identify_consumer(const openlcb::EventRegistryEntry &registry_entry,
                                      openlcb::EventReport *event,
                                                                        BarrierNotifiable *done)
{
}
void NetworkHealthScan::NetworkHealthConsumer::handle_event_report(const openlcb::EventRegistryEntry &entry,
                                 openlcb::EventReport *event,
                                                                   BarrierNotifiable *done)
{
}
void NetworkHealthScan::NetworkHealthConsumer::register_handler()
{
}
void NetworkHealthScan::NetworkHealthConsumer::unregister_handler()
{
}
void NetworkHealthScan::NetworkHealthConsumer::SendAllConsumersIdentified(openlcb::EventReport *event,
                                                                          BarrierNotifiable *done)
{
}
void NetworkHealthScan::NetworkHealthConsumer::SendConsumerIdentified(openlcb::EventReport *event,
                                                                      BarrierNotifiable *done)
{
}


void NetworkHealthScan::NetworkHealthProducer::handle_identify_global(const openlcb::EventRegistryEntry &registry_entry, 
                                    openlcb::EventReport *event, 
                                                                      BarrierNotifiable *done)
{
}

void NetworkHealthScan::NetworkHealthProducer::handle_identify_producer(const openlcb::EventRegistryEntry &registry_entry,
                                      openlcb::EventReport *event, 
                                                                        BarrierNotifiable *done)
{
}

void NetworkHealthScan::NetworkHealthProducer::register_handler()
{
}
void NetworkHealthScan::NetworkHealthProducer::unregister_handler()
{
}
void SendAllProducersIdentified(openlcb::EventReport *event,
                                BarrierNotifiable *done)
{
}
void NetworkHealthScan::NetworkHealthProducer::SendProducerIdentified(openlcb::EventReport *event,
                                                                      BarrierNotifiable *done)
{
}
void NetworkHealthScan::NetworkHealthProducer::SendEventReport(openlcb::WriteHelper *helper,
                             openlcb::EventId event,
                                                               BarrierNotifiable *done)
{
}

ConfigUpdateListener::UpdateAction NetworkHealthScan::apply_configuration(int fd,
                                             bool initial_load,
                                                                                  BarrierNotifiable *done)
{
    return UPDATED;
}
void NetworkHealthScan::factory_reset(int fd)
{
}
void NetworkHealthScan::handle_identify_producer(const openlcb::EventRegistryEntry &registry_entry,
                                  openlcb::EventReport *event, 
                                                 BarrierNotifiable *done)
{
}

void NetworkHealthScan::browseCallback_(openlcb::NodeID nodeid)
{
    auto found = NodeDB_.find(nodeid);
    if (found == NodeDB_.end())
    {
        auto request = invoke_flow(&snipClient_,node_,openlcb::NodeHandle(nodeid));
        string manufacturer("");
        string model("");
        string softwareVersion("");
        string hardwareVersion("");
        string name("");
        string description("");
        size_t index = 0;
        openlcb::Payload p = request->data()->response;
        uint8_t version = p[index++];
        for (size_t i=0; i<version; i++)
        {
            char c = p[index++];
            while (c != '\0')
            {
                switch (i)
                {
                case 0:
                    manufacturer += c;
                    break;
                case 1:
                    model += c;
                    break;
                case 2:
                    softwareVersion += c;
                    break;
                case 3:
                    hardwareVersion += c;
                    break;
                }
                c = p[index++];
            }
        }
        version = p[index++];
        for (size_t i=0; i<version; i++)
        {
            char c = p[index++];
            while (c != '\0')
            {
                switch (i)
                {
                case 0:
                    name += c;
                    break;
                case 1:
                    description += c;
                    break;
                }
                c = p[index++];
            }
        }
        NodeDB_.insert(
            std::make_pair(
                 nodeid,
                 NetworkNodeDatabaseEntry(nodeid,manufacturer,
                                          model,softwareVersion,
                                          hardwareVersion,name,
                                          description,
                                          NetworkNodeDatabaseEntry::New)));
    }
    else
    {
        found->second.status = NetworkNodeDatabaseEntry::Found;
    }
}


void NetworkHealthScan::resetNodeDB()
{
}
void NetworkHealthScan::ReadDB_()
{
}

void NetworkHealthScan::WriteD_B()
{
}

NetworkHealthScan::ScanStatus_t NetworkHealthScan::scanNetwork_()
{
    return OK;
}

}
