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
#  Last Modified : <240910.2051>
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
    def cutRJ45s(self,part):
        r1BB=self.RJ45_1.BoundBox
        r1Cube=Part.makeBox(r1BB.XLength,r1BB.YLength,r1BB.ZLength,\
                            Base.Vector(r1BB.XMin,r1BB.YMin,r1BB.ZMin),\
                            Base.Vector(0,0,1))
        part = part.cut(r1Cube)
        r2BB=self.RJ45_2.BoundBox
        r2Cube=Part.makeBox(r2BB.XLength,r2BB.YLength,r2BB.ZLength,\
                            Base.Vector(r2BB.XMin,r2BB.YMin,r2BB.ZMin),\
                            Base.Vector(0,0,1))
        part = part.cut(r2Cube)
        return part
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

def printEdge(e,f):
    print(e)
    print(e.Curve,file=f)
    for v in e.Vertexes:
        print(v.Point,file=f)
    print(e.FirstParameter,file=f)
    print(e.LastParameter,file=f)

class Box(object):
    __InnerLength=99
    __InnerWidth=62.04
    __InnerDepth=33.77
    __WallThick=3.5
    __CornerRad=4
    __YOffset=15.156
    __USBCutY=85.344
    __PostRadious=3
    __PostHoleRadious=1.025
    __PostHoleDepth=12.5
    __LidDip=12.5
    __LidZ0=3.5
    __LidZLen=25.4
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector")
        self.origin = origin
        self.display = Adafruit.AdafruitTFTFeatherWing("display",self.origin)
        self.feather=Adafruit.AdafruitFeather("board",\
                            self.origin.add(Base.Vector(1.6+7.37,\
                                                        39.7-5.08,\
                                                        17.526+(2.54))))
        self.featherLCCCAN=FeatherLCCCAN("lccCAN",\
                            self.origin.add(Base.Vector(((1.6+7.37)*2)+2.155,\
                                                        39.7-5.08+50.8,\
                                                        17.70+(2.54))))
        self.makeBox()
        self.makeLid()
    def makeBox(self):
        outerLength=self.__InnerLength+(self.__WallThick*2)
        outerWidth=self.__InnerWidth+(self.__WallThick*2)
        extrude=Base.Vector(-(self.__InnerDepth+self.__WallThick),0,0)
        baseNorm=Base.Vector(1,0,0)
        baseOrigin=self.origin.add(Base.Vector(-extrude.x,\
                                               -self.__YOffset,\
                                               -self.__WallThick))
        elist=list()
        p1=baseOrigin.add(Base.Vector(0,self.__CornerRad,0))
        p2=baseOrigin.add(Base.Vector(0,outerLength-self.__CornerRad,0))
        elist.append(Part.makeLine(p1,p2))
        c1=p2.add(Base.Vector(0,0,self.__CornerRad))
        elist.append(Part.makeCircle(self.__CornerRad,c1,Base.Vector(1,0,0),\
                                     270,360))
        p3=c1.add(Base.Vector(0,self.__CornerRad,0))
        p4=p3.add(Base.Vector(0,0,outerWidth-self.__CornerRad))
        elist.append(Part.makeLine(p3,p4))
        c2=p4.add(Base.Vector(0,-self.__CornerRad,0))
        elist.append(Part.makeCircle(self.__CornerRad,c2,Base.Vector(1,0,0),\
                                    0,90))
        p5=c2.add(Base.Vector(0,0,self.__CornerRad))
        p6=p5.add(Base.Vector(0,-(outerLength-(2*self.__CornerRad)),0))
        elist.append(Part.makeLine(p5,p6))
        c3=p6.add(Base.Vector(0,0,-self.__CornerRad))
        elist.append(Part.makeCircle(self.__CornerRad,c3,Base.Vector(1,0,0),\
                                    90,180))
        p7=c3.add(Base.Vector(0,-self.__CornerRad,0))
        p8=p7.add(Base.Vector(0,0,-(outerWidth-self.__CornerRad)))
        elist.append(Part.makeLine(p7,p8))
        c4=p8.add(Base.Vector(0,self.__CornerRad,0))
        elist.append(Part.makeCircle(self.__CornerRad,c4,Base.Vector(1,0,0),\
                                    180,270))
        elist = Part.__sortEdges__(elist)
        baseOutline=Part.Wire(elist)
        baseFace=Part.Face(baseOutline)
        box=baseFace.extrude(extrude)
        elist=list()
        iextrude=extrude.add(Base.Vector(-self.__WallThick,0,0))
        ip1=p1.add(Base.Vector(-self.__WallThick,\
                               self.__WallThick,\
                               self.__WallThick))
        ip2=p2.add(Base.Vector(-self.__WallThick,\
                               -self.__WallThick,\
                               self.__WallThick))
        
        elist.append(Part.makeLine(ip1,ip2)) 
        ic1=ip2.add(Base.Vector(0,0,self.__CornerRad))
        elist.append(Part.makeCircle(self.__CornerRad,ic1,Base.Vector(1,0,0),\
                                     270,360))
        ip3=ic1.add(Base.Vector(0,self.__CornerRad,0))
        ip4=ip3.add(Base.Vector(0,0,self.__InnerWidth-self.__CornerRad))
        elist.append(Part.makeLine(ip3,ip4))
        ic2=ip4.add(Base.Vector(0,-self.__CornerRad,0))
        elist.append(Part.makeCircle(self.__CornerRad,ic2,Base.Vector(1,0,0),\
                                     0,90))
        ip5=ic2.add(Base.Vector(0,0,self.__CornerRad))
        ip6=ip5.add(Base.Vector(0,-(self.__InnerLength-(2*self.__CornerRad)),0))
        elist.append(Part.makeLine(ip5,ip6))
        ic3=ip6.add(Base.Vector(0,0,-self.__CornerRad))
        elist.append(Part.makeCircle(self.__CornerRad,ic3,Base.Vector(1,0,0),\
                                     90,180))
        ip7=ic3.add(Base.Vector(0,-self.__CornerRad,0))
        ip8=ip7.add(Base.Vector(0,0,-(self.__InnerWidth-self.__CornerRad)))
        elist.append(Part.makeLine(ip7,ip8))
        ic4=ip8.add(Base.Vector(0,self.__CornerRad,0))
        elist.append(Part.makeCircle(self.__CornerRad,ic4,Base.Vector(1,0,0),\
                                     180,270))
        elist = Part.__sortEdges__(elist)
        innerOutline=Part.Wire(elist)
        innerFace=Part.Face(innerOutline)
        inner=innerFace.extrude(iextrude)
        box=box.cut(inner)
        box=box.cut(self.feather.USB_Cutout(self.__USBCutY,self.__WallThick))
        box=self.featherLCCCAN.cutRJ45s(box)
        box=box.fuse(self.display.MakeStandoff(0,self.origin.x+1.6,\
                                               self.__InnerDepth,\
                                               self.__PostRadious,\
                                               self.__PostHoleRadious,\
                                               self.__PostHoleDepth))
        box=box.fuse(self.display.MakeStandoff(1,self.origin.x+1.6,\
                                               self.__InnerDepth,\
                                               self.__PostRadious,\
                                               self.__PostHoleRadious,\
                                               self.__PostHoleDepth))
        box=box.fuse(self.display.MakeStandoff(2,self.origin.x+1.6,\
                                               self.__InnerDepth,\
                                               self.__PostRadious,\
                                               self.__PostHoleRadious,\
                                               self.__PostHoleDepth))
        box=box.fuse(self.display.MakeStandoff(3,self.origin.x+1.6,\
                                               self.__InnerDepth,\
                                               self.__PostRadious,\
                                               self.__PostHoleRadious,\
                                               self.__PostHoleDepth))
        box=self.display.CutBoard(box)
        self.box = box
    def makeLid(self):
        outerLength=self.__InnerLength+(self.__WallThick*2)
        outerWidth=self.__InnerWidth+(self.__WallThick*2)
        extrude=Base.Vector(-self.__WallThick,0,0)
        lidNorm=Base.Vector(1,0,0)
        lidOrigin=self.origin.add(Base.Vector(0,\
                                               -self.__YOffset,\
                                               -self.__WallThick))
        elist=list()
        p1=lidOrigin.add(Base.Vector(0,self.__CornerRad,0))
        p2=lidOrigin.add(Base.Vector(0,outerLength-self.__CornerRad,0))
        elist.append(Part.makeLine(p1,p2))
        c1=p2.add(Base.Vector(0,0,self.__CornerRad))
        elist.append(Part.makeCircle(self.__CornerRad,c1,Base.Vector(1,0,0),\
                                     270,360))
        p3=c1.add(Base.Vector(0,self.__CornerRad,0))
        p4=p3.add(Base.Vector(0,0,outerWidth-self.__CornerRad))
        elist.append(Part.makeLine(p3,p4))
        c2=p4.add(Base.Vector(0,-self.__CornerRad,0))
        elist.append(Part.makeCircle(self.__CornerRad,c2,Base.Vector(1,0,0),\
                                    0,90))
        p5=c2.add(Base.Vector(0,0,self.__CornerRad))
        p6=p5.add(Base.Vector(0,-(outerLength-(2*self.__CornerRad)),0))
        elist.append(Part.makeLine(p5,p6))
        c3=p6.add(Base.Vector(0,0,-self.__CornerRad))
        elist.append(Part.makeCircle(self.__CornerRad,c3,Base.Vector(1,0,0),\
                                    90,180))
        p7=c3.add(Base.Vector(0,-self.__CornerRad,0))
        p8=p7.add(Base.Vector(0,0,-(outerWidth-self.__CornerRad)))
        elist.append(Part.makeLine(p7,p8))
        c4=p8.add(Base.Vector(0,self.__CornerRad,0))
        elist.append(Part.makeCircle(self.__CornerRad,c4,Base.Vector(1,0,0),\
                                    180,270))
        elist = Part.__sortEdges__(elist)
        lidOutline=Part.Wire(elist)
        lidFace=Part.Face(lidOutline)
        lid=lidFace.extrude(extrude)
        lid=lid.cut(self.display.ScreenCutout(extrude.x))
        lid=lid.cut(self.display.MakeMountingHole(0,lidOrigin.x,extrude.x))
        lid=lid.cut(self.display.MakeMountingHole(1,lidOrigin.x,extrude.x))
        lid=lid.cut(self.display.MakeMountingHole(2,lidOrigin.x,extrude.x))
        lid=lid.cut(self.display.MakeMountingHole(3,lidOrigin.x,extrude.x))
        dipNorm=Base.Vector(0,-1,0)
        extrude=Base.Vector(0,self.__WallThick,0)
        elist=list()
        p1=self.origin.add(Base.Vector(0,-self.__YOffset,self.__LidZ0))
        p2=p1.add(Base.Vector(self.__LidDip,0,0))
        elist.append(Part.makeLine(p1,p2))
        c1=p2.add(Base.Vector(0,0,self.__CornerRad))
        elist.append(Part.makeCircle(self.__CornerRad,c1,dipNorm,270,360))
        p3=c1.add(Base.Vector(self.__CornerRad,0,0))
        p4=p3.add(Base.Vector(0,0,self.__LidZLen))
        elist.append(Part.makeLine(p3,p4))
        c2=p4.add(Base.Vector(-self.__CornerRad,0,0))
        elist.append(Part.makeCircle(self.__CornerRad,c2,dipNorm,0,90))
        p5=c2.add(Base.Vector(0,0,self.__CornerRad))
        p6=p5.add(Base.Vector(-self.__LidDip,0,0))
        elist.append(Part.makeLine(p5,p6))
        elist.append(Part.makeLine(p6,p1))
        elist = Part.__sortEdges__(elist)
        dipOutline=Part.Wire(elist)
        dipFace=Part.Face(dipOutline)
        dip=dipFace.extrude(extrude)
        lid=lid.fuse(dip)
        self.box=self.box.cut(lid)
        self.lid=lid
    def ExportSTLs(self,boxfilename,lidfilename):
        objs=[]
        if "PrintBox" in App.listDocuments().keys():
            App.closeDocument("PrintBox")
        doc = App.newDocument("PrintBox")
        obj = doc.addObject("Part::Feature","temp")
        b = self.box.copy()
        b = b.rotate(Base.Vector(0,0,0),Base.Vector(0,1,0),90)
        obj.Shape=b
        objs.append(obj)
        Gui.activeDocument().activeView().viewTop()
        Gui.SendMsgToActiveView("ViewFit")
        Mesh.export(objs,boxfilename)
        if "PrintLid" in App.listDocuments().keys():
            App.closeDocument("PrintLid")
        doc = App.newDocument("PrintLid")
        objs=[]
        obj = doc.addObject("Part::Feature","temp")
        obj.Shape=self.lid.copy().rotate(Base.Vector(0,0,0),Base.Vector(0,1,0),-90)
        objs.append(obj)
        Gui.activeDocument().activeView().viewTop()
        Gui.SendMsgToActiveView("ViewFit")
        Mesh.export(objs,lidfilename)
    def show(self,doc=None):
        if doc==None:
            doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name+'_box')
        obj.Shape = self.box
        obj.Label=self.name+'_box'
        obj.ViewObject.ShapeColor=tuple([1.0,0.0,0.0])
        obj.ViewObject.Transparency=50
        obj = doc.addObject("Part::Feature",self.name+'_lid')
        obj.Shape = self.lid
        obj.Label=self.name+'_lid'
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,1.0])
        obj.ViewObject.Transparency=50
        self.display.show(doc)
        self.feather.show(doc)
        self.featherLCCCAN.show(doc)
    

if __name__ == '__main__':
    if "DisplayCase" in App.listDocuments().keys():
        App.closeDocument("DisplayCase")
    doc = App.newDocument("DisplayCase")
    box=Box("box",Base.Vector(0,0,0))
    box.show(doc)
    Gui.activeDocument().activeView().viewRear()
    Gui.SendMsgToActiveView("ViewFit")
    box.ExportSTLs("Box.stl","Lid.stl")
