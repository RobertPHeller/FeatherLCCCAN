##-*- makefile -*-############################################################
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Tue Sep 3 16:51:09 2024
#  Last Modified : <240904.1431>
#
#  Description	
#
#  Notes
#
#  History
#	
#  $Log$
#
##############################################################################
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
##############################################################################


set(deps
    driver
    heap
    nvs_flash
    vfs
    app_update
    spiffs
    OpenMRNIDF
    Config
    NVSManager
    FS
    HealthMonitor
    RebootHelpers
    json
)

idf_component_register(SRCS esp32networkhealth.cpp NetworkHealthScan.cpp
		       REQUIRES "${deps}")
		       

# export the project version as a define for the SNIP data, note it must be
# truncated to 21 characters max.
idf_build_get_property(project_ver PROJECT_VER)
string(SUBSTRING "${project_ver}" 0 20 SNIP_SW_VERSION)
idf_build_get_property(project_name PROJECT_NAME)
string(SUBSTRING "${project_name}" 0 40 SNIP_PROJECT_NAME)

set(SNIP_HW_VERSION "1.0.0")
set(SNIP_PROJECT_PAGE "https://www.deepsoft.com/")
set(CDI_VERSION "0x0100")

set_source_files_properties(esp32networkhealth.cpp PROPERTIES COMPILE_FLAGS "-Wno-error=maybe-uninitialized -Wno-implicit-fallthrough -Wno-ignored-qualifiers")
set_source_files_properties(NetworkHealthScan.cpp PROPERTIES COMPILE_FLAGS "-Wno-error=maybe-uninitialized -Wno-implicit-fallthrough -Wno-ignored-qualifiers")
set_source_files_properties(esp32networkhealth.cpp PROPERTIES COMPILE_DEFINITIONS "SNIP_PROJECT_PAGE=\"${SNIP_PROJECT_PAGE}\"; SNIP_HW_VERSION=\"${SNIP_HW_VERSION}\"; SNIP_SW_VERSION=\"${SNIP_SW_VERSION}\"; SNIP_PROJECT_NAME=\"${SNIP_PROJECT_NAME}\"; CDI_VERSION=${CDI_VERSION}")

set(CDIDEPS
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/Config/include/cdi.hxx
    ${CMAKE_CURRENT_SOURCE_DIR}/../../.git/index
    ${CMAKE_CURRENT_SOURCE_DIR}/../sdkconfig
   )


add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/../components/Config/include/cdidata.hxx" 
                          "${CMAKE_CURRENT_SOURCE_DIR}/../components/Config/include/Revision.hxxout"
                   COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/../components/Config/CDI 
                   DEPENDS "${CDIDEPS}"
                   VERBATIM
                  )

add_custom_target(FORCE 
                  ALL
                  DEPENDS                                                     
                  "${CMAKE_CURRENT_SOURCE_DIR}/../components/Config/include/cdidata.hxx"
                 )
