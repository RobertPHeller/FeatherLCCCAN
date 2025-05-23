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
//  Last Modified : <250314.1730>
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
#include "StringUtils.hxx"
#include "openlcb/RefreshLoop.hxx"
#include "openlcb/SimpleStack.hxx"
#include "executor/Timer.hxx"
#include "executor/Notifiable.hxx"
#include "openlcb/NodeBrowser.hxx"
#include "openlcb/SNIPClient.hxx"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "utils/logging.h"
#include <string>
#include "NetworkHealthScanConfig.hxx"
#include "NetworkHealthScan.hxx"
#include "hardware.hxx"
#include "Esp32SPI.hxx"
extern Esp32SPI spibus;
extern Esp32SPI::SpiDevice *sdcard;

namespace NetworkHealthScan
{

void NetworkHealthScan::NetworkHealthConsumer::handle_identify_global(const openlcb::EventRegistryEntry &registry_entry, 
                                                                      openlcb::EventReport *event, 
                                                                      BarrierNotifiable *done)
{
    if (event->dst_node && event->dst_node != node_)
    {
        return done->notify();
    }
    SendAllConsumersIdentified(event, done);
    done->maybe_done();
}

void NetworkHealthScan::NetworkHealthConsumer::handle_identify_consumer(const openlcb::EventRegistryEntry &registry_entry,
                                                                        openlcb::EventReport *event,
                                                                        BarrierNotifiable *done)
{
    if (event->dst_node && event->dst_node != node_)
    {
        return done->notify();
    }
    event->event_write_helper<1>()->WriteAsync(node_,
                                               openlcb::Defs::MTI_CONSUMER_IDENTIFIED_UNKNOWN,
                                               openlcb::WriteHelper::global(),
                                               openlcb::eventid_to_buffer(event->event),
                                               done);
}
void NetworkHealthScan::NetworkHealthConsumer::handle_event_report(const openlcb::EventRegistryEntry &entry,
                                                                   openlcb::EventReport *event,
                                                                   BarrierNotifiable *done)
{
    if (event->event == scan_)
    {
        parent_->ScanNetwork();
    }
    else if (event->event == resetList_)
    {
        parent_->ResetNodeDB();
    }
    done->notify();
}
void NetworkHealthScan::NetworkHealthConsumer::register_handler()
{
    openlcb::EventRegistry::instance()->register_handler(
                openlcb::EventRegistryEntry(this, scan_, 0), 0);
    openlcb::EventRegistry::instance()->register_handler(
                openlcb::EventRegistryEntry(this, resetList_, 0), 0);
}
void NetworkHealthScan::NetworkHealthConsumer::unregister_handler()
{
    openlcb::EventRegistry::instance()->unregister_handler(this);
}
void NetworkHealthScan::NetworkHealthConsumer::SendAllConsumersIdentified(openlcb::EventReport *event,
                                                                          BarrierNotifiable *done)
{
    openlcb::Defs::MTI mti = openlcb::Defs::MTI_CONSUMER_IDENTIFIED_UNKNOWN;
    event->event_write_helper<3>()->WriteAsync(node_,mti,
           openlcb::WriteHelper::global(), openlcb::eventid_to_buffer(scan_), done->new_child());
    event->event_write_helper<4>()->WriteAsync(node_,mti,
           openlcb::WriteHelper::global(), openlcb::eventid_to_buffer(resetList_), done->new_child());
}


void NetworkHealthScan::NetworkHealthProducer::handle_identify_global(const openlcb::EventRegistryEntry &registry_entry, 
                                    openlcb::EventReport *event, 
                                                                      BarrierNotifiable *done)
{
    if (event->dst_node && event->dst_node != node_)
    {
        return done->notify();
    }
    SendAllProducersIdentified(event, done);
    done->maybe_done();
}

void NetworkHealthScan::NetworkHealthProducer::handle_identify_producer(const openlcb::EventRegistryEntry &registry_entry,
                                      openlcb::EventReport *event, 
                                                                        BarrierNotifiable *done)
{
    if (event->dst_node && event->dst_node != node_)
    {
        return done->notify();
    }
    event->event_write_helper<1>()->WriteAsync(node_,
                                               openlcb::Defs::MTI_PRODUCER_IDENTIFIED_UNKNOWN,
                                               openlcb::WriteHelper::global(),
                                               openlcb::eventid_to_buffer(event->event),
                                               done);
}

void NetworkHealthScan::NetworkHealthProducer::register_handler()
{
    openlcb::EventRegistry::instance()->register_handler(
                openlcb::EventRegistryEntry(this, scanOK_, 0), 0);
    openlcb::EventRegistry::instance()->register_handler(
                openlcb::EventRegistryEntry(this, scanMissing_, 0), 0);
    openlcb::EventRegistry::instance()->register_handler(
                openlcb::EventRegistryEntry(this, scanAdded_, 0), 0);
}
void NetworkHealthScan::NetworkHealthProducer::unregister_handler()
{
    openlcb::EventRegistry::instance()->unregister_handler(this);
}
void NetworkHealthScan::NetworkHealthProducer::SendAllProducersIdentified(openlcb::EventReport *event,
                                                                          BarrierNotifiable *done)
{
    openlcb::Defs::MTI mti = openlcb::Defs::MTI_PRODUCER_IDENTIFIED_UNKNOWN;
    event->event_write_helper<2>()->WriteAsync(node_,mti,
         openlcb::WriteHelper::global(), openlcb::eventid_to_buffer(scanOK_), done->new_child());
    event->event_write_helper<3>()->WriteAsync(node_,mti,
         openlcb::WriteHelper::global(), openlcb::eventid_to_buffer(scanMissing_), done->new_child());
    event->event_write_helper<4>()->WriteAsync(node_,mti,
         openlcb::WriteHelper::global(), openlcb::eventid_to_buffer(scanAdded_), done->new_child());
    
}
void NetworkHealthScan::NetworkHealthProducer::SendEventReport(openlcb::WriteHelper *helper,
                                                               openlcb::EventId event,
                                                               BarrierNotifiable *done)
{
    helper->WriteAsync(node_,  openlcb::Defs::MTI_EVENT_REPORT, 
                       openlcb::WriteHelper::global(), 
                       openlcb::eventid_to_buffer(event), done);
}

ConfigUpdateListener::UpdateAction NetworkHealthScan::apply_configuration(int fd,
                                                                          bool initial_load,
                                                                          BarrierNotifiable *done)
{
    UpdateAction result = UPDATED;
    AutoNotify n(done);
    openlcb::EventId cfg_scan = cfg_.scan().read(fd);
    openlcb::EventId cfg_resetList = cfg_.resetList().read(fd);
    openlcb::EventId cfg_scanOK = cfg_.scanOK().read(fd);
    openlcb::EventId cfg_scanMissing = cfg_.scanMissing().read(fd);
    openlcb::EventId cfg_scanAdded = cfg_.scanAdded().read(fd);
    if (consumer_.eventsChanged(cfg_scan,cfg_resetList))
    {
        consumer_.~NetworkHealthConsumer();
        new (&consumer_) NetworkHealthConsumer(node_,this,cfg_scan,cfg_resetList);
        result = REINIT_NEEDED; // Causes events identify.
    }
    if (producer_.eventsChanged(cfg_scanOK,cfg_scanMissing,cfg_scanAdded))
    {
        producer_.~NetworkHealthProducer();
        new (&producer_) NetworkHealthProducer(node_,this,cfg_scanOK,cfg_scanMissing,cfg_scanAdded);
        result = REINIT_NEEDED; // Causes events identify.
    }
    return result;
}
void NetworkHealthScan::factory_reset(int fd)
{
}

StateFlowBase::Action NetworkHealthScan::BrowseHandleFlow::entry()
{
    LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow] entry()");
    if (!pendingNodeIDs_.empty())
    {
        PendingNodeID *temp = (PendingNodeID*) (pendingNodeIDs_.next().item);
        LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow] entry(): temp = %p",temp);
        openlcb::NodeID nodeid = temp->nodeid;
        delete temp;
        LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow] entry(): pendingNodeIDs_ contains %d items",pendingNodeIDs_.pending());
        auto found = parent_->NodeDB_Find(nodeid);
        if (found == parent_->NodeDB_End())
        {
            snipHelper.SNIPAsync(&snipProcess_,node_,openlcb::NodeHandle(nodeid),
                                 this);
            busy_ = true;
            return wait_and_call(STATE(gotSNIP));
        }
        else
        {
            return again();
        }
    }
    else
    {
        return wait();
    }
}

StateFlowBase::Action NetworkHealthScan::BrowseHandleFlow::gotSNIP()
{
    LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow] gotSNIP()]");
    if (busy_) return wait();
    if (pendingNodeIDs_.empty())
    {
        return wait_and_call(STATE(entry));
    }
    else
    {
        return call_immediately(STATE(entry));
    }
}

void NetworkHealthScan::BrowseHandleFlow::browseCallback_(openlcb::NodeID nodeid)
{
    LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow] browseCallback_(0x%012llX)",nodeid);
    //LOG(INFO,"[NetworkHealthScan] browseCallback_(), currentState_ is %d",currentState_);
    auto found = parent_->NodeDB_Find(nodeid);
    if (found == parent_->NodeDB_End())
    {
        LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow] browseCallback_(): found == NodeDB_.end()");
        //SyncNotifiable n;
        //snipHelper.SNIPAsync(&snipProcess_,node_,openlcb::NodeHandle(nodeid),
        //                     this);
        //n.wait_for_notification();
        QMember *newitem = new PendingNodeID(nodeid);
        LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow] browseCallback_(): newitem is %p",newitem);
        pendingNodeIDs_.insert(newitem);
        LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow] browseCallback_(): pendingNodeIDs_ countains %d elements",pendingNodeIDs_.pending());
        if (!busy_) notify();
    }
    else
    {
        LOG(INFO,"[NetworkHealthScan] browseCallback_(): found != NodeDB_.end()");
        found->second.status = NetworkNodeDatabaseEntry::Found;
    }
}


void NetworkHealthScan::ResetNodeDB()
{
    LOG(INFO, "[NetworkHealthScan] ResetNodeDB()");
    if (currentState_ == Scanning) return;
    remove(NODEDB);
    NodeDB_.clear();
    needWriteDB_ = true;
    ScanNetwork();
}

static bool readline_to_string(int fd,string& buffer)
{
    char c;
    int count = 0;
    buffer = "";
    while (read(fd,&c,1) > 0)
    {
        if (c == '\n') return true;
        buffer += c;
        count++;
    }
    return count > 0;
}

void NetworkHealthScan::ReadDB_()
{
    NodeDB_.clear();
    int fd = open(NODEDB,O_RDONLY);
    if (fd < 0)
    {
        needWriteDB_ = true;
        return;
    }
    string nodestring;
    while (readline_to_string(fd,nodestring))
    {
        
        openlcb::NodeID nodeid = utils::string_to_uint64(nodestring);
        string manufacturer; readline_to_string(fd,manufacturer);
        string model; readline_to_string(fd,model);
        string softwareVersion; readline_to_string(fd,softwareVersion);
        string hardwareVersion; readline_to_string(fd,hardwareVersion);
        string name; readline_to_string(fd,name);
        string description; readline_to_string(fd,description);
        NodeDB_.insert(
            std::make_pair(
                 nodeid,
                 NetworkNodeDatabaseEntry(nodeid,manufacturer,
                                          model,softwareVersion,
                                          hardwareVersion,name,
                                          description,
                                          NetworkNodeDatabaseEntry::Missing)));
    }
    close(fd);
}


void NetworkHealthScan::WriteDB_()
{
    remove(NODEDB);
    
    int fd = creat(NODEDB,0777);
    int err = errno;
    LOG(INFO,"[NetworkHealthScan] WriteDB_(): fd = %d, err is %d",fd,err);
    LOG(INFO,"[NetworkHealthScan] WriteDB_(): NODEDB is '%s'",NODEDB);
    for (auto it = NodeDB_.begin(); it != NodeDB_.end(); it++)
    {
        string nodeidstring = utils::node_id_to_string(it->second.node_id);
        write(fd,nodeidstring.c_str(),nodeidstring.size());
        write(fd,"\n",1);
        write(fd,it->second.manufacturer.c_str(),it->second.manufacturer.size());
        write(fd,"\n",1);
        write(fd,it->second.model.c_str(),it->second.model.size());
        write(fd,"\n",1);
        write(fd,it->second.softwareVersion.c_str(),it->second.softwareVersion.size());
        write(fd,"\n",1);
        write(fd,it->second.hardwareVersion.c_str(),it->second.hardwareVersion.size());
        write(fd,"\n",1);
        write(fd,it->second.name.c_str(),it->second.name.size());
        write(fd,"\n",1);
        write(fd,it->second.description.c_str(),it->second.description.size());
        write(fd,"\n",1);
    }
    close(fd);
    spibus.unmount_sd_card(sdcard);
    sdcard = spibus.mount_sd_card("/sdcard",CONFIG_CardCS);
}

void NetworkHealthScan::ScanNetwork()
{
    LOG(INFO, "[NetworkHealthScan] ScanNetwork()");
    if (currentState_ == Scanning) return;
    for (auto it = NodeDB_.begin(); it != NodeDB_.end(); it++)
    {
        it->second.status = NetworkNodeDatabaseEntry::Missing;
    }
    currentState_ = Scanning;
    browsehandleflow_.refresh();
    start(BROWSETIMEOUT);
}

long long NetworkHealthScan::timeout()
{
    LOG(INFO, "[NetworkHealthScan] timeout()");
    currentState_ = ScanComplete;
    found_ = 0;
    missing_ = 0;
    added_ = 0;
    for (auto it = NodeDB_.begin(); it != NodeDB_.end(); it++)
    {
        switch (it->second.status)
        {
        case NetworkNodeDatabaseEntry::Missing:
            missing_++;
            break;
        case NetworkNodeDatabaseEntry::Found:
            found_++;
            break;
        case NetworkNodeDatabaseEntry::New:
            added_++;
            break;
        }
    }
    LOG(INFO, "[NetworkHealthScan] timeout(): Total() = %d", Total());
    bn_.reset(this);
    if (found_ == Total())
    {
        LOG(INFO, "[NetworkHealthScan] timeout(): found_ = %d", found_);
        producer_.sendOK(&write_helpers[0],bn_.new_child());
    }
    if (missing_ > 0)
    {
        LOG(INFO, "[NetworkHealthScan] timeout(): missing_ = %d", missing_);
        producer_.sendMissing(&write_helpers[1],bn_.new_child());
    }
    if (added_ > 0)
    {
        LOG(INFO, "[NetworkHealthScan] timeout(): added_ = %d", added_);
        producer_.sendAdded(&write_helpers[2],bn_.new_child());
    }
    bn_.maybe_done();
    LOG(INFO, "[NetworkHealthScan] timeout(): needWriteDB_ = %d", needWriteDB_);
    if (needWriteDB_)
    {
        WriteDB_();
        needWriteDB_ = false;
    }
    return NONE;
}

StateFlowBase::Action NetworkHealthScan::BrowseHandleFlow::SNIPProcess::entry()
{
    LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow::SNIPProcess::entry()]");
    return allocate_and_call(&client_,STATE(startSNIP));
}

StateFlowBase::Action NetworkHealthScan::BrowseHandleFlow::SNIPProcess::startSNIP()
{
    LOG(INFO,"[NetworkHealthScan::BrowseHandleFlow::SNIPProcess::startSNIP()]");
    buffer_ = get_allocation_result(&client_);
    GetSNIP *m = message()->data();
    buffer_->data()->reset(m->src,m->dst);
    buffer_->data()->done.reset(this);
    client_.send(buffer_);
    return wait_and_call(STATE(gotSNIP));
}

StateFlowBase::Action NetworkHealthScan::BrowseHandleFlow::SNIPProcess::gotSNIP()
{
    string manufacturer("");
    string model("");
    string softwareVersion("");
    string hardwareVersion("");
    string name("");
    string description("");
    size_t index = 0;
    GetSNIP *m = message()->data();
    LOG(INFO,"[NetworkHealthScan] SNIPProcess::gotSNIP(): buffer_->data()->resultCode is %d",buffer_->data()->resultCode);
    openlcb::Payload p = buffer_->data()->response;
    LOG(INFO,"[NetworkHealthScan] SNIPProcess::gotSNIP(): p = %s",p.c_str());
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
    LOG(INFO,"[NetworkHealthScan] SNIPProcess::gotSNIP(): version = %d",version);
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
    LOG(INFO,"[NetworkHealthScan] SNIPProcess::gotSNIP(): 0X%012llX: manufacturer is '%s', model = '%s', softwareVersion = '%s', hardwareVersion = '%s', name = '%s', description = '%s'",m->dst.id,manufacturer.c_str(),model.c_str(),softwareVersion.c_str(),hardwareVersion.c_str(),name.c_str(),description.c_str());
    parent_->insertDB(m->dst.id,
                      NetworkNodeDatabaseEntry(m->dst.id,manufacturer,
                                               model,softwareVersion,
                                               hardwareVersion,name,
                                               description,
                                               NetworkNodeDatabaseEntry::New));
    

    buffer_->unref();
    buffer_ = nullptr;
    *busy_ = false;
    return release_and_exit();
}

}
