
#include "Channel.h"

#include <fstream>
#include <sstream>

ClassImp(Channel)

std::unordered_map<int, Channel*> Channel::fChannelMap;

//============== Channel ==============
// Purpose: Create a channel from one definition line.
// Inputs: Space-separated channel definition.
// Outputs: Initialized channel object.
Channel::Channel(std::string line)
{
  Set(line);
}

//============== Read ==============
// Purpose: Read channel definitions from a calibration file.
// Inputs: Calibration file path.
// Outputs: Populated static channel map.
void Channel::Read(std::string fFile)
{
  std::ifstream file(fFile.c_str());
  std::string line;
  Channel* c = nullptr;
  if(fChannelMap.empty()) {
    c = new Channel("dummy 999 0xffff 0 1 0");
    fChannelMap[c->fAddress] = c;
  }
  while(getline(file, line)) {
    c = new Channel(line);
    fChannelMap[c->fAddress] = c;
  }
}

//============== Set ==============
// Purpose: Parse one channel definition.
// Inputs: Space-separated channel definition.
// Outputs: Updated channel fields.
void Channel::Set(std::string line)
{
  std::stringstream ss(line);
  ss >> fName;
  ss >> fNumber;
  ss >> std::hex >> fAddress;
  double temp;
  while(ss >> temp) {
    fCalPars.push_back(temp);
  }
}

//============== Get ==============
// Purpose: Find a channel by address.
// Inputs: Numeric channel address.
// Outputs: Matching channel or the fallback channel.
Channel* Channel::Get(int address)
{
  if(fChannelMap.count(address) != 0u) {
    return fChannelMap[address];
  }
  return fChannelMap[0xffff];
}

//============== Print ==============
// Purpose: Print channel metadata and calibration parameters.
// Inputs: Unused ROOT option.
// Outputs: Text written to standard output.
void Channel::Print(Option_t*) const
{
  printf("channel[0x%x]\n", fAddress);
  printf("\tName:     %s\n", fName.c_str());
  printf("\tNumber:   %i\n", fNumber);
  printf("\tCal Pars: ");
  for(auto parameter : fCalPars) {
    printf("%.04f  ", parameter);
  }
  printf("\n");
}
