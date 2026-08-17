#this will compile the current library.
add_library(OUTPUTMANAGER SHARED OutputManager.cxx)

target_link_libraries(OUTPUTMANAGER PRIVATE
  PHYSICS
  TMIDAS
  ROOT::Core
  ROOT::RIO
  ROOT::Tree)

#link the compiled library above to the final executable.
target_link_libraries(s2426Sort PRIVATE OUTPUTMANAGER)
