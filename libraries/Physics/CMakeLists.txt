
#this will compile the current library.
add_library(PHYSICS SHARED 
  DetectorEvent.cxx
  Tigress.cxx
  Emma.cxx
)

target_include_directories(PHYSICS PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)

root_generate_dictionary(G__PHYSICS 
  DetectorEvent.h
  Tigress.h
  Emma.h
  MODULE PHYSICS 
  LINKDEF Linkdef.h
)

target_link_libraries(PHYSICS
  PUBLIC ROOT::Physics
  PRIVATE TMIDAS CHANNEL ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(s2426Sort PRIVATE PHYSICS)        
