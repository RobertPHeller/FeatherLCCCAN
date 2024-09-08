#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Tue Aug 13 18:16:55 2024
#  Last Modified : <240908.0926>
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

class AdafruitTFTFeatherWing(object):
    __BoardOutlineYZInches = [(0,0),(.2,0),(.2,.2),(3.36-.2,.2),(3.36-.2,0),\
                              (3.36,0),(3.36,2.6),(3.36-.2,2.6),\
                              (3.36-.2,2.6-.2),(.2,2.6-.2),(.2,2.6),(0,2.6),\
                              (0,0)]
    __BoardWidthInches = 3.36
    __BoardThick = 1.6
    __BoardMountingHolesYZInches = [(.1,.1),(3.26,.1),(3.26,2.5),(.1,2.5)]
    __BoardMountingHolesRad = .06*25.4
    __LongHeaderYZ = (39.7,40.386)
    __LongHeaderWH = (40.64,8.128)
    __ShortHeaderYZ = (39.7,17.526)
    __ShortHeaderWH = (30.48,8.128)
    __HeaderHeight = 7.37
    __ScreenOriginYZInches = (0,.2)
    __ScreenSizeWHInches   = (3.36,2.2)
    __ScreenThick = 3.3
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector")
        self.origin = origin
        boardPoints = list()
        for Yin,Zin in self.__BoardOutlineYZInches:
            boardPoints.append(origin.add(Base.Vector(0,Yin*25.4,Zin*25.4)))
        self.board = Part.Face(Part.makePolygon(boardPoints))\
                        .extrude(Base.Vector(self.__BoardThick,0,0))
        self.MountingHoles = list()
        for Yin,Zin in self.__BoardMountingHolesYZInches:
            self.MountingHoles.append(origin.add(Base.Vector(0,Yin*25.4,Zin*25.4)))
        #self.holes = list()
        for i in range(0,4):
            h = self.MakeMountingHole(i,origin.x,self.__BoardThick)
            #self.holes.append(h)
            self.board = self.board.cut(h)
        screenWin,screenHin = self.__ScreenSizeWHInches
        screenYin,screenZin = self.__ScreenOriginYZInches
        self.screen = Part.makePlane(screenHin*25.4,screenWin*25.4,\
                                     origin.add(Base.Vector(0,screenWin*25.4+screenYin*25.4,screenZin*25.4)),\
                                     Base.Vector(1,0,0)).extrude(Base.Vector(-self.__ScreenThick,0,0))
        Y,Z = self.__LongHeaderYZ
        W,H = self.__LongHeaderWH
        self.longHeader =  Part.makePlane(H,W,\
                                          origin.add(Base.Vector(self.__BoardThick,Y+W,Z)),\
                                          Base.Vector(1,0,0))\
                               .extrude(Base.Vector(self.__HeaderHeight))
        Y,Z = self.__ShortHeaderYZ
        W,H = self.__ShortHeaderWH
        self.shortHeader =  Part.makePlane(H,W,\
                                          origin.add(Base.Vector(self.__BoardThick,Y+W,Z)),\
                                          Base.Vector(1,0,0))\
                               .extrude(Base.Vector(self.__HeaderHeight))
    def MakeMountingHole(self,index,X,Xdelta):
        holeOrig = Base.Vector(X,self.MountingHoles[index].y,self.MountingHoles[index].z)
        hole = Part.Face(Part.Wire(Part.makeCircle(self.__BoardMountingHolesRad,\
                                                   holeOrig,\
                                                   Base.Vector(1,0,0))))\
                    .extrude(Base.Vector(Xdelta,0,0))
        return hole
    def MakeStandoff(self,index,X,Xdelta,radius,holerad=0,holedepth=0):
        orig = Base.Vector(X,self.MountingHoles[index].y,self.MountingHoles[index].z)
        standoff = Part.Face(Part.Wire(Part.makeCircle(radius,\
                                                       orig,\
                                                       Base.Vector(1,0,0))))\
                    .extrude(Base.Vector(Xdelta,0,0))
        if holerad > 0:
            if holedepth==0:
                holedepth=Xdelta
            hole = Part.Face(Part.Wire(Part.makeCircle(holerad,\
                                                       orig,\
                                                       Base.Vector(1,0,0))))\
                     .extrude(Base.Vector(holedepth,0,0))
        return standoff.cut(hole)
    def ScreenCutout(self,DeltaX):
        screenWin,screenHin = self.__ScreenSizeWHInches
        screenYin,screenZin = self.__ScreenOriginYZInches
        screenCutout = Part.makePlane(screenHin*25.4,screenWin*25.4,\
                                     self.origin.add(Base.Vector(0,screenWin*25.4+screenYin*25.4,screenZin*25.4)),\
                                     Base.Vector(1,0,0)).extrude(Base.Vector(DeltaX,0,0))
        return screenCutout
    def CutBoard(self,part):
        return part.cut(self.board)
    def show(self,doc=None):
        if doc==None:
            doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name+'_board')
        obj.Shape = self.board
        obj.Label=self.name+'_board'
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        # for h in self.holes:
        # obj = doc.addObject("Part::Feature",self.name+'_hole')
        # obj.Shape = h
        # obj.ViewObject.ShapeColor=tuple([1.0,0.0,0.0]) 
        obj = doc.addObject("Part::Feature",self.name+'_screen')
        obj.Shape = self.screen
        obj.Label=self.name+'_screen'
        obj.ViewObject.ShapeColor=tuple([0.8,0.8,0.8])
        obj = doc.addObject("Part::Feature",self.name+'_longHeader')
        obj.Shape = self.longHeader
        obj.Label=self.name+'_longHeader'
        obj.ViewObject.ShapeColor=tuple([0.2,0.2,0.2])
        obj = doc.addObject("Part::Feature",self.name+'_shortHeader')
        obj.Shape = self.shortHeader
        obj.Label=self.name+'_shortHeader'
        obj.ViewObject.ShapeColor=tuple([0.2,0.2,0.2])
        

class AdafruitFeather(object):
    #__metaclass__ = ABCMeta
    __p1=Base.Vector(0,2.54,0)
    __p2=Base.Vector(0,48.26,0)
    __cornerRad=2.54
    __c1=Base.Vector(0,48.26,2.54)
    __p3=Base.Vector(0,50.80, 2.54)
    __p4=Base.Vector(0,50.80, 20.32)
    __c2=Base.Vector(0,48.26, 20.32)
    __p5=Base.Vector(0,48.26, 22.86)
    __p6=Base.Vector(0,2.54, 22.86)
    __c3=Base.Vector(0,2.54, 20.32)
    __p7=Base.Vector(0,0, 20.32)
    __p8=Base.Vector(0,0, 2.54)
    __c4=Base.Vector(0, 2.54, 2.54)
    __boardThick=1.6
    __XHoleSpacing=(1.8*25.4)
    __BoardLength=(2*25.4)
    __ZHoleSpacing=(.75*25.4)
    __BoardWidth=(.9*25.4)
    __H1=Base.Vector(0, 2.54, 2.54)
    __H2=Base.Vector(0, 2.54, 20.32)
    __H12Rad=1.25
    __H3=Base.Vector(0, 48.26, 1.8415)
    __H4=Base.Vector(0, 48.26, 20.955)
    __H34Rad=0.5754
    __PinL1=Base.Vector(0, 6.35, 1.27)
    __LPinCount=16
    __PinS1=Base.Vector(0,16.51, 21.59)
    __SPinCount=12
    __PinDelta=Base.Vector(0,2.54,0)
    __PinRad=.5
    __USB_CutRad=3
    __USB_Z0 = 7.146478+1.5
    __USB_Z1 = 16.052787-1.5
    __USB_X0 = 3.346186
    __USB_XA = 0.346186
    __USB_XB = 6.346186
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector")
        self.origin = origin
        self.makeBoard()
        self.makeHeaders()
        self.makeUSBConnector()
    def makeBoard(self):
        elist=list()
        elist.append(Part.makeLine(self.origin.add(self.__p1),self.origin.add(self.__p2)))
        elist.append(Part.makeCircle(self.__cornerRad,self.origin.add(self.__c1),\
                         Base.Vector(1,0,0),270,360))
        elist.append(Part.makeLine(self.origin.add(self.__p3),self.origin.add(self.__p4)))
        elist.append(Part.makeCircle(self.__cornerRad,self.origin.add(self.__c2),\
                        Base.Vector(1,0,0),0,90))
        elist.append(Part.makeLine(self.origin.add(self.__p5),self.origin.add(self.__p6)))
        elist.append(Part.makeCircle(self.__cornerRad,self.origin.add(self.__c3),\
                        Base.Vector(1,0,0),90,180))
        elist.append(Part.makeLine(self.origin.add(self.__p7),self.origin.add(self.__p8)))
        elist.append(Part.makeCircle(self.__cornerRad,self.origin.add(self.__c4),\
                        Base.Vector(1,0,0),180,270))
        #boardOutline=Part.makeCompound(elist)
        #self.board=boardOutline.extrude(Base.Vector(self.__boardThick,0,0))
        elist = Part.__sortEdges__(elist)
        boardOutline=Part.Wire(elist)
        boardFace=Part.Face(boardOutline)
        board=boardFace.extrude(Base.Vector(self.__boardThick,0,0))
        H1 = Part.Face(Part.Wire(Part.makeCircle(self.__H12Rad,\
                                                 self.origin.add(self.__H1),\
                                                 Base.Vector(1,0,0))))\
                      .extrude(Base.Vector(self.__boardThick,0,0))
        board=board.cut(H1)
        H2 = Part.Face(Part.Wire(Part.makeCircle(self.__H12Rad,\
                                                 self.origin.add(self.__H2),\
                                                 Base.Vector(1,0,0))))\
                      .extrude(Base.Vector(self.__boardThick,0,0))
        board=board.cut(H2)
        H3 = Part.Face(Part.Wire(Part.makeCircle(self.__H34Rad,\
                                                 self.origin.add(self.__H3),\
                                                 Base.Vector(1,0,0))))\
                      .extrude(Base.Vector(self.__boardThick,0,0))
        board=board.cut(H3)
        H4 = Part.Face(Part.Wire(Part.makeCircle(self.__H34Rad,\
                                                 self.origin.add(self.__H4),\
                                                 Base.Vector(1,0,0))))\
                      .extrude(Base.Vector(self.__boardThick,0,0))
        board=board.cut(H4)
        pinl=self.origin.add(self.__PinL1)
        for i in range(0,self.__LPinCount):
            pin=Part.Face(Part.Wire(Part.makeCircle(self.__PinRad,pinl,Base.Vector(1,0,0))))\
                        .extrude(Base.Vector(self.__boardThick,0,0))
            board=board.cut(pin)
            pinl=pinl.add(self.__PinDelta)
        pins=self.origin.add(self.__PinS1)
        for i in range(0,self.__SPinCount):
            pin=Part.Face(Part.Wire(Part.makeCircle(self.__PinRad,pins,Base.Vector(1,0,0))))\
                        .extrude(Base.Vector(self.__boardThick,0,0))
            board=board.cut(pin)
            pins=pins.add(self.__PinDelta)
        self.__center=self.origin.add(Base.Vector(0,self.__BoardLength/2,\
                                                  self.__BoardWidth/2))
        self.board=board.rotate(self.__center,Base.Vector(1,0,0),180)
    def makeHeaders(self):
        shorthead = Mesh.read(os.path.join(os.path.dirname(__file__),\
                              "PinSocket_1x12_P254mm_Vertical.smf"))
        shorthead.rotate(0,3.14159/2,0)
        shorthead.translate(self.origin.x+self.__boardThick,\
                            self.origin.y+34.2125,\
                            self.origin.z+1.27)
        self.shorthead = shorthead
        longhead = Mesh.read(os.path.join(os.path.dirname(__file__),\
                              "PinSocket_1x16_P254mm_Vertical.smf"))
        longhead.rotate(0,3.14159/2,0)
        longhead.translate(self.origin.x+self.__boardThick,\
                           self.origin.y+44.3725,\
                           self.origin.z+21.625)
        self.longhead = longhead
    def makeUSBConnector(self):
        usbC = Mesh.read(os.path.join(os.path.dirname(__file__),\
                        "USB_C_Receptacle_GCT_USB4105-xx-A_16P_TopMnt_Horizontal.smf"))
        usbC.translate(4.451228,3.634496,0)
        usbC.rotate(0,0,3.14159)
        usbC.rotate(0,3.14159/2,0)
        usbC.translate(self.origin.x+self.__boardThick,\
                       self.origin.y+50.8+1.457,\
                       self.origin.z+15.75-8.620426)
        self.usbC = usbC
    def USB_Cutout(self,Y0,Thick):
        elist=list()
        oXAZ0 = self.origin.add(Base.Vector(self.__USB_XA,0,self.__USB_Z0))
        oXAZ1 = self.origin.add(Base.Vector(self.__USB_XA,0,self.__USB_Z1))
        elist.append(Part.makeLine(Base.Vector(oXAZ0.x,Y0,oXAZ0.z),\
                                   Base.Vector(oXAZ1.x,Y0,oXAZ1.z)))
        oXBZ0 = self.origin.add(Base.Vector(self.__USB_XB,0,self.__USB_Z0))
        oXBZ1 = self.origin.add(Base.Vector(self.__USB_XB,0,self.__USB_Z1))
        elist.append(Part.makeLine(Base.Vector(oXBZ0.x,Y0,oXBZ0.z),\
                                   Base.Vector(oXBZ1.x,Y0,oXBZ1.z)))
        oX0Z0 = self.origin.add(Base.Vector(self.__USB_X0,0,self.__USB_Z0))
        elist.append(Part.makeCircle(self.__USB_CutRad,\
                                     Base.Vector(oX0Z0.x,Y0,oX0Z0.z),\
                                     Base.Vector(0,1,0),0,180))
        oX0Z1 = self.origin.add(Base.Vector(self.__USB_X0,0,self.__USB_Z1))
        elist.append(Part.makeCircle(self.__USB_CutRad,\
                                     Base.Vector(oX0Z1.x,Y0,oX0Z1.z),\
                                     Base.Vector(0,1,0),180,360))
        elist = Part.__sortEdges__(elist)
        cutoutOutline=Part.Wire(elist)
        cutoutFace=Part.Face(cutoutOutline)
        cutout=cutoutFace.extrude(Base.Vector(0,Thick,0))
        return cutout
    def show(self,doc=None):
        if doc==None:
            doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name+'_board')
        obj.Shape = self.board
        obj.Label=self.name+'_board'
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        obj = doc.addObject("Mesh::Feature",self.name+'_shorthead')
        obj.Mesh = self.shorthead
        obj.Label=self.name+'_shorthead'
        obj.ViewObject.ShapeColor=tuple([1.0,0.0,0.0])
        obj = doc.addObject("Mesh::Feature",self.name+'_longhead')
        obj.Mesh = self.longhead
        obj.Label=self.name+'_longhead'
        obj.ViewObject.ShapeColor=tuple([0.0,1.0,0.0])
        obj = doc.addObject("Mesh::Feature",self.name+'_usbC')
        obj.Mesh = self.usbC
        obj.Label=self.name+'_usbC'
        obj.ViewObject.ShapeColor=tuple([0.8,0.8,0.8])


if __name__ == '__main__':
    if "Display" in App.listDocuments().keys():
        App.closeDocument("Display")
    doc = App.newDocument("Display")
    display=AdafruitTFTFeatherWing("display",Base.Vector(0,0,0))
    #display.show(doc)
    feather=AdafruitFeather("board",Base.Vector(1.6+7.37,39.7-5.08,\
                                                17.526+(2.54*1.75)))
    feather.show(doc)
    Gui.activeDocument().activeView().viewRight()
    Gui.SendMsgToActiveView("ViewFit")
    
