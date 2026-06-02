#ifdef __ROOTCLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Fragment+;
#pragma link C++ class Event+;
#pragma link C++ class TigressHit+;
#pragma link C++ class Tigress+;
#pragma link C++ class TEmma+;
#pragma link C++ class std::vector<Fragment>+;
#pragma link C++ class std::vector<TigressHit>+;
#pragma link C++ class std::vector<size_t>+;
#pragma link C++ class std::map<int,std::vector<size_t> >+;
#pragma link C++ class std::map<int,std::vector<Fragment> >+;

#endif
