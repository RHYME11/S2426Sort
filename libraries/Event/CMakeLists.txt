
#this will compile the current library.
add_library(EVENT SHARED Event.cxx)
root_generate_dictionary(G__Event Event.h MODULE EVENT LINKDEF LinkDef.h)
target_link_libraries(EVENT PRIVATE TMIDAS ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(s2426Sort EVENT)
