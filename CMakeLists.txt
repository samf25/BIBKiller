########################################################
# cmake file for building Gaudi BIB SoftKiller Package
# @author Samuel Ferraro
########################################################

### DATA ###########################################################

### LIBRARY #################################################################

# Main library with Processors
FILE( GLOB _plugin_sources src/*.cxx )
gaudi_add_module(BIBKillPlugins
		SOURCES		${_plugin_sources}
		LINK		Gaudi::GaudiKernel
        			EDM4HEP::edm4hep
        			ROOT::Hist
        			${ROOT_LIBRARIES}
        			k4FWCore::k4FWCore )

INSTALL( TARGETS BIBKillPlugins
	EXPORT ${CMAKE_PROJECT_NAME}Targets
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
	PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/@{CMAKE_PROJECT_NAME}"
	COMPONENT dev )

TARGET_INCLUDE_DIRECTORIES(BIBKillPlugins
		PRIVATE	./BIBKiller
			${CMAKE_CURRENT_BINARY_DIR}
			${Gaudi_INCLUDE_DIRS}
			${ROOT_INCLUDE_DIRS})

SET( GAUDI_GENCONF_DIR "genConfDir" )
if (${Gaudi_VERSION} VERSION_LESS 35.1)
	SET( GAUDI_GENCONF_DIR "genConf" )
endif()
