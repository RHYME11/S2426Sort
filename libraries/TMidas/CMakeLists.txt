
#this will compile the current library.
add_library(TMIDAS SHARED TMidasEvent.cxx  TMidasFile.cxx  TXMLOdb.cxx)


root_generate_dictionary(G__TMIDAS 
  TMidasBanks.h
  TMidasEvent.h
  TMidasEventHeader.h
  TMidasFile.h
  TRawEvent.h
  TRawFile.h
  MODULE TMIDAS LINKDEF LinkDef.h)

target_link_libraries(TMIDAS ROOT::Core ROOT::MathCore CHANNEL DATA)

#link the compiled libraries above to the final executable.
target_link_libraries(s2426Sort TMIDAS)        
target_link_libraries(analysis TMIDAS)
