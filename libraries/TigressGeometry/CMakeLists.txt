
add_library(TIGRESSGEOMETRY SHARED
  TigressGeometry.cxx)

root_generate_dictionary(G__TigressGeometry
  TigressGeometry.h
  MODULE TIGRESSGEOMETRY
  LINKDEF LinkDef.h)

target_link_libraries(TIGRESSGEOMETRY PUBLIC ${ROOT_LIBRARIES})
