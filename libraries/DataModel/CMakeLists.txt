
#this will compile the current library.
add_library(DATA SHARED
  ../TMidas/Fragment.cxx
  ../Event/Event.cxx
  ../Event/TigressHit.cxx
  ../Event/Tigress.cxx
  ../Event/Emma.cxx)

root_generate_dictionary(G__Data
  Fragment.h
  Event.h
  TigressHit.h
  Tigress.h
  Emma.h
  MODULE DATA
  LINKDEF LinkDef.h)

target_link_libraries(DATA PUBLIC CHANNEL ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(s2426Sort DATA)
target_link_libraries(analysis DATA)
