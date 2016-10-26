cmake_minimum_required (VERSION 2.6)
project (AliAna)

include_directories(
  ${CMAKE_SOURCE_DIR}
  $ENV{ROOTSYS}/include
  $ENV{ALICE_ROOT}/include
  $ENV{ALICE_PHYSICS}/include
  )

set (CMAKE_CXX_FLAGS "-std=c++11")

macro(generate_dictionary DNAME LDNAME DHDRS DINCDIRS)
  foreach( dir ${DINCDIRS})
    set(INCLUDE_PATH -I${dir} ${INCLUDE_PATH})
  endforeach()

  separate_arguments(EXTRADEFINITIONS UNIX_COMMAND "${ARGV4}")

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lib${DNAME}.rootmap ${CMAKE_CURRENT_BINARY_DIR}/G__${DNAME}.cxx ${CMAKE_CURRENT_BINARY_DIR}/G__${DNAME}_rdict.pcm
    COMMAND rootcling
    ARGS
    -f ${CMAKE_CURRENT_BINARY_DIR}/G__${DNAME}.cxx
    -rmf ${CMAKE_CURRENT_BINARY_DIR}/lib${DNAME}.rootmap -rml lib${DNAME}
    ${GLOBALDEFINITIONS} ${EXTRADEFINITIONS} ${INCLUDE_PATH} ${DHDRS} ${LDNAME}
    DEPENDS ${DHDRS} ${LDNAME} ${ROOT_CINT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/lib${DNAME}.rootmap" DESTINATION lib)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/G__${DNAME}_rdict.pcm" DESTINATION lib)
endmacro(generate_dictionary)

get_directory_property(incdirs INCLUDE_DIRECTORIES)

set(HDRS
  src/AliAnaSelector.h
  src/AliAnaQA.h
  )

generate_dictionary("Analysis" "src/Analysis_LinkDef.h" "${HDRS}" "${incdirs}" "-rml libTreePlayer")
add_library(Analysis SHARED src/AliAnaSelector.cxx src/AliAnaQA.cxx G__Analysis.cxx)
install (TARGETS Analysis DESTINATION lib)

set(HDRS
  src/plotter.h
  )
generate_dictionary("Plot" "src/Plot_LinkDef.h" "${HDRS}" "${incdirs}")
add_library(Plot SHARED src/plotter.cxx G__Plot.cxx)
install (TARGETS Plot DESTINATION lib)

install(FILES
  src/ana.C
  src/plot.C
  DESTINATION macros
  )

install(FILES
  src/run.sh
  DESTINATION bin
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
  )
