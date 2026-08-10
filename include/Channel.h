#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "TObject.h"

class Channel : public TObject {
public:
  Channel() = default;
  ~Channel() override = default;

  static void Read(std::string fFile);
  void Set(std::string fLine);

  void Print(Option_t* option = "") const override;

  static Channel* Get(int address);

  std::string Name() const { return fName; }
  int Number() const { return fNumber; }
  std::vector<double> CalPars() const { return fCalPars; }

private:
  explicit Channel(std::string line);

  std::string fName{""};
  int fNumber{-1};
  int fAddress{-1};
  std::vector<double> fCalPars;

  static std::unordered_map<int, Channel*> fChannelMap; //!

  ClassDefOverride(Channel, 1)
};

#endif
