#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Fri Sep 6 15:36:22 2024
#  Last Modified : <240907.1506>
#
#  Description	
#
#  Notes
#
#  History
#	
#*****************************************************************************
#
#    Copyright (C) 2024  Robert Heller D/B/A Deepwoods Software
#			51 Locke Hill Road
#			Wendell, MA 01379-9728
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# 
#
#*****************************************************************************


import FreeCAD as App
import Part, Mesh
from FreeCAD import Base

import os
import sys
sys.path.append(os.path.dirname(__file__))

from abc import ABCMeta, abstractmethod, abstractproperty

import Adafruit

class FeatherLCCCAN(object):
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector")
        self.origin = origin
        self.makeBoard()
        self.makeRJ45s()
        self.makePinHeaders()
    def makeBoard(self):
        board=Mesh.read(os.path.join(os.path.dirname(__file__),\
                        "FeatherLCCCAN.smf"))
        board.translate(-123.4222,+68.796715,0)
        board.rotate(0,3.14159/2,0)
        board.rotate(-3.14159/2,0,0)
        board.translate(self.origin.x,\
                        self.origin.y,\
                        self.origin.z)
        self.board=board
    def makeRJ45s(self):
        RJ45_1=Mesh.read(os.path.join(os.path.dirname(__file__),\
                        "WurthElektronik-615008144221.smf"))
        RJ45_1.translate(8.92,6.13,1.232)
        RJ45_1.rotate(3.14159/2,0,0)
        RJ45_1.rotate(0,3.14159/2,0)
        RJ45_1.translate(self.origin.x+1.27+1.6,\
                         self.origin.y-60.37,\
                         self.origin.z+20.223)
        self.RJ45_1 = RJ45_1
        RJ45_2=Mesh.read(os.path.join(os.path.dirname(__file__),\
                        "WurthElektronik-615008144221.smf"))
        RJ45_2.translate(8.92,6.13,1.232)
        RJ45_2.rotate(3.14159/2,0,0)
        RJ45_2.rotate(0,3.14159/2,0)
        RJ45_2.translate(self.origin.x+1.27+1.6,\
                         self.origin.y-80.77,\
                         self.origin.z+20.223)
        
        self.RJ45_2 = RJ45_2
    def makePinHeaders(self):
        shortheader=Mesh.read(os.path.join(os.path.dirname(__file__),\
                              "PinHeader_1x12_p254mm_Vertical.smf"))
        shortheader.translate(1.23887,29.1,0)
        shortheader.rotate(0,-3.14159/2,0)
        shortheader.translate(self.origin.x,\
                              self.origin.y-45.5,\
                              self.origin.z-.15)
        self.shortheader = shortheader
        longheader=Mesh.read(os.path.join(os.path.dirname(__file__),\
                             "PinHeader_1x16_p254mm_Vertical.smf"))
        longheader.translate(1.23887,39.346,0)
        longheader.rotate(0,-3.14159/2,0)
        longheader.translate(self.origin.x,\
                             self.origin.y-45.5,\
                             self.origin.z+20)
        self.longheader = longheader
    def show(self,doc=None):
        if doc==None:
            doc = App.activeDocument()
        obj = doc.addObject("Mesh::Feature",self.name+'_board')
        obj.Mesh = self.board
        obj.Label=self.name+'_board'
        obj.ViewObject.ShapeColor=tuple([0.0,1.0,0.0])
        obj = doc.addObject("Mesh::Feature",self.name+'_rj45_1')
        obj.Mesh = self.RJ45_1
        obj.Label=self.name+'_rj45_1'
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        obj = doc.addObject("Mesh::Feature",self.name+'_rj45_2')
        obj.Mesh = self.RJ45_2
        obj.Label=self.name+'_rj45_2'
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        obj = doc.addObject("Mesh::Feature",self.name+'_shortheader')
        obj.Mesh = self.shortheader
        obj.Label=self.name+'_shortheader'
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        obj = doc.addObject("Mesh::Feature",self.name+'_longheader')
        obj.Mesh = self.longheader
        obj.Label=self.name+'_longheader'
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])

if __name__ == '__main__':
    if "DisplayCase" in App.listDocuments().keys():
        App.closeDocument("DisplayCase")
    doc = App.newDocument("DisplayCase")
    display = Adafruit.AdafruitTFTFeatherWing("display",Base.Vector(0,0,0))
    #display.show(doc)
    feather=Adafruit.AdafruitFeather("board",Base.Vector(1.6+7.37,39.7-5.08,17.526+(2.54*1.75)))
    feather.show(doc)
    featherLCCCAN=FeatherLCCCAN("lccCAN",Base.Vector(((1.6+7.37)*2)+2.155,39.7-5.08+50.8,17.65+(2.54*1.78)))
    featherLCCCAN.show(doc)
    Gui.activeDocument().activeView().viewRear()
    Gui.SendMsgToActiveView("ViewFit")
