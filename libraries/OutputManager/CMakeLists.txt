
#this will compile the current library.
add_library(OUTPUTMANAGER SHARED OutputManager.cxx)
target_link_libraries(OUTPUTMANAGER PRIVATE DATA ROOT::Core ROOT::RIO ROOT::Tree)

#link the compiled libraries above to the final executable.
target_link_libraries(s2426Sort OUTPUTMANAGER)
target_link_libraries(analysis OUTPUTMANAGER)
