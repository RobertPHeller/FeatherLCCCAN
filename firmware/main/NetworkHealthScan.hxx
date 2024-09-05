// -!- c++ -!- //////////////////////////////////////////////////////////////
//
//  System        : 
//  Module        : 
//  Object Name   : $RCSfile$
//  Revision      : $Revision$
//  Date          : $Date$
//  Author        : $Author$
//  Created By    : Robert Heller
//  Created       : Wed Sep 4 12:44:50 2024
//  Last Modified : <240904.2157>
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

#ifndef __NETWORKHEALTHSCAN_HXX
#define __NETWORKHEALTHSCAN_HXX

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
#include <map>
#include <algorithm>
#include <functional>
#include "NetworkHealthScanConfig.hxx"

namespace NetworkHealthScan
{
struct NetworkNodeDatabaseEntry {
    typedef enum {Missing=0, Found, New} Status_t;
    const openlcb::NodeID node_id;
    string manufacturer;
    string model;
    string softwareVersion;
    string hardwareVersion;
    string name;
    string description;
    Status_t status;
    NetworkNodeDatabaseEntry(openlcb::NodeID node_id_=0,
                             string manufacturer_="",
                             string model_="",
                             string softwareVersion_="",
                             string hardwareVersion_="",
                             string name_="",
                             string description_="",
                             Status_t status_=Missing)
                : node_id(node_id_)
          , manufacturer(manufacturer_)
          , model(model_)
          , softwareVersion(softwareVersion_)
          , hardwareVersion(hardwareVersion_)
          , name(name_)
          , description(description_)
          , status(status_)
    {
    }
    NetworkNodeDatabaseEntry(const NetworkNodeDatabaseEntry& other)
                : node_id(other.node_id)
          , manufacturer(other.manufacturer)
          , model(other.model)
          , softwareVersion(other.softwareVersion)
          , hardwareVersion(other.hardwareVersion)
          , name(other.name)
          , description(other.description)
          , status(other.status)
    {
    }
};



class NetworkHealthScan : public DefaultConfigUpdateListener, public Timer
{
private:
    class NetworkHealthConsumer : public openlcb::SimpleEventHandler
    {
    public:
        NetworkHealthConsumer(openlcb::Node *node,
                              NetworkHealthScan *parent,
                              openlcb::EventId scan, 
                              openlcb::EventId resetList)
                    : node_(node)
              , parent_(parent)
              , scan_(scan)
              , resetList_(resetList)
        {
            register_handler();
        }
        ~NetworkHealthConsumer()
        {
            unregister_handler();
        }
        bool eventsChanged(openlcb::EventId newscan, 
                           openlcb::EventId newresetList)
        {
            return (newscan != scan_ || newresetList != resetList_);
        }
        void handle_identify_global(const openlcb::EventRegistryEntry &registry_entry, 
                                    openlcb::EventReport *event, 
                                    BarrierNotifiable *done);
        void handle_identify_consumer(const openlcb::EventRegistryEntry &registry_entry,
                                      openlcb::EventReport *event,
                                      BarrierNotifiable *done);
        void handle_event_report(const openlcb::EventRegistryEntry &entry,
                                 openlcb::EventReport *event,
                                 BarrierNotifiable *done);
    private:
        void register_handler();
        void unregister_handler();
        void SendAllConsumersIdentified(openlcb::EventReport *event,
                                        BarrierNotifiable *done);
        openlcb::Node *node_;
        NetworkHealthScan *parent_;
        openlcb::EventId scan_;
        openlcb::EventId resetList_;
    };
    class NetworkHealthProducer : public openlcb::SimpleEventHandler
    {
    public:
        NetworkHealthProducer(openlcb::Node *node,
                              NetworkHealthScan *parent,
                              openlcb::EventId scanOK,
                              openlcb::EventId scanMissing,
                              openlcb::EventId scanAdded)
                    : node_(node)
              , parent_(parent)
              , scanOK_(scanOK)
              , scanMissing_(scanMissing)
              , scanAdded_(scanAdded)
        {
            register_handler();
        }
        ~NetworkHealthProducer()
        {
            unregister_handler();
        }
        bool eventsChanged(openlcb::EventId newscanOK, 
                           openlcb::EventId newscanMissing,
                           openlcb::EventId newscanAdded)
        {
            return (newscanOK != scanOK_ || newscanMissing != scanMissing_ ||
                    newscanAdded != scanAdded_);
        }
        void handle_identify_global(const openlcb::EventRegistryEntry &registry_entry, 
                                    openlcb::EventReport *event, 
                                    BarrierNotifiable *done);
        void handle_identify_producer(const openlcb::EventRegistryEntry &registry_entry,
                                      openlcb::EventReport *event, 
                                      BarrierNotifiable *done);
        void sendOK(openlcb::WriteHelper *helper, BarrierNotifiable *done)
        {
            SendEventReport(helper,scanOK_,done);
        }
        void sendMissing(openlcb::WriteHelper *helper, BarrierNotifiable *done)
        {
            SendEventReport(helper,scanMissing_,done);
        }
        void sendAdded(openlcb::WriteHelper *helper, BarrierNotifiable *done)
        {
            SendEventReport(helper,scanAdded_, done);
        }
        
    private:
        void register_handler();
        void unregister_handler();
        void SendAllProducersIdentified(openlcb::EventReport *event,
                                        BarrierNotifiable *done);
        void SendEventReport(openlcb::WriteHelper *helper,
                             openlcb::EventId event,
                             BarrierNotifiable *done);
        openlcb::Node *node_;
        NetworkHealthScan *parent_;
        
        openlcb::EventId scanOK_;
        openlcb::EventId scanMissing_;
        openlcb::EventId scanAdded_;
    };
public:
    NetworkHealthScan(openlcb::Node *node, Service *service, 
                      ActiveTimers *timers,
                      const NetworkHealthScanConfig &cfg)
                : Timer(timers)
          , node_(node)
          , service_(service)
          , cfg_(cfg)
          , browser_(node, std::bind(&NetworkHealthScan::browseCallback_,
                                     this,std::placeholders::_1))
          , snipClient_(service)
          , consumer_(node,this,0,0)
          , producer_(node,this,0,0,0)
          , needWriteDB_(false)
    {
        ReadDB_();
    }
    ~NetworkHealthScan() 
    {
    }
    virtual UpdateAction apply_configuration(int fd,
                                             bool initial_load,
                                             BarrierNotifiable *done) override;
    virtual void factory_reset(int fd) override;
private:
    static constexpr const char NODEDB[] = "/fs/nodedb";
    friend class NetworkHealthConsumer;
    void browseCallback_(openlcb::NodeID nodeid);
    void resetNodeDB_();
    typedef std::map<openlcb::NodeID,NetworkNodeDatabaseEntry> NodeDB_t;
    NodeDB_t NodeDB_;
    void ReadDB_();
    void WriteDB_();
    typedef enum {OK=0, MISSING, ADDED} ScanStatus_t;
    void scanNetwork_();
    long long timeout() override;
    openlcb::Node *node_;
    Service *service_;
    const NetworkHealthScanConfig cfg_;
    openlcb::NodeBrowser browser_;
    openlcb::SNIPClient snipClient_;
    NetworkHealthConsumer consumer_;
    NetworkHealthProducer producer_;
    BarrierNotifiable bn_;
    bool needWriteDB_;
};
          
          
}
#endif // __NETWORKHEALTHSCAN_HXX

