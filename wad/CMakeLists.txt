file(GLOB_RECURSE ODAWAD_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/wadinfo.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap/doom2.wad
  ${CMAKE_CURRENT_SOURCE_DIR}/flats/-noflat-.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/*.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/lumps/*.lmp
  ${CMAKE_CURRENT_SOURCE_DIR}/sprites/*.gif)

if (USE_EXTERNAL_DEUTEX)
  find_program(DEUTEX_EXECUTABLE deutex
    PATHS "/usr/games") # Debian installation location
elseif(TARGET deutex)
  set(DEUTEX_EXECUTABLE deutex)
endif()

# If DeuTex is available, use it to build the WAD.
if(DEUTEX_EXECUTABLE)
  message(STATUS "Found DeuTex: ${DEUTEX_EXECUTABLE}")

  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/odamex.wad"
    COMMAND "${DEUTEX_EXECUTABLE}" -overwrite -rgb 0 255 255 -doom2 bootstrap -build wadinfo.txt "${CMAKE_CURRENT_BINARY_DIR}/odamex.wad"
    DEPENDS ${ODAWAD_SOURCES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM)

  add_custom_target(odawad ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/odamex.wad")

  if(WIN32)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/odamex.wad"
      DESTINATION .
      COMPONENT common)
  else()
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/odamex.wad"
      DESTINATION "${CMAKE_INSTALL_DATADIR}/odamex"
      COMPONENT common)
  endif()
else()
  message(WARNING "Could NOT find DeuTex, ODAMEX.WAD will not be built.")
endif()
