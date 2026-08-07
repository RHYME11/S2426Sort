// TXMLOdb.h TMidasEvent.h TMidasFile.h
#ifdef __ROOTCLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef HAS_XML
#pragma link C++ class TXMLOdb+;
#endif
#pragma link C++ class TMidasEvent+;
#pragma link C++ class TMidasFile+;

#pragma link C++ class TRawEvent+;
#pragma link C++ class TRawFile+;

#pragma link C++ class Fragment+;
#pragma link C++ class std::vector<Fragment>+;

#endif
