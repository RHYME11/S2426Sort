#include <DetectorProcess.h>

#include <Emma.h>
#include <Histogramer.h>
#include <OutputManager.h>
#include <Tigress.h>

#include <cmath>

DetectorProcess *DetectorProcess::fDetectorProcess = 0;

DetectorProcess::DetectorProcess() {
  fWorker = std::thread([this]{ this->loop(); });
  fWorker.detach();
}

DetectorProcess *DetectorProcess::Get() {
  if(!fDetectorProcess)
    fDetectorProcess = new DetectorProcess;
  return fDetectorProcess;
}

DetectorProcess::~DetectorProcess() {
  printf("DetectorProcess destructor called; fStop[%i]\n",fStop.load());
  if(!fStop)
    fWorker.join();
}

// ============== loop ==============
// Purpose: Build detector physics, fill histograms, and write Physics trees.
// Inputs: Detector events from EventProcess.
// Outputs: Completed histogram and Physics tree entries.
void DetectorProcess::loop() {
  while(1) {
    if(fStop) break;

    DetectorEvent event;
    if(!EventProcess::Get()->pop(event)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    for(const auto& frag : event.Fragments()) {
      Histogramer::Fill("DetectorType",100,0,100,frag.DetType());
    }

    Tigress tigress;
    Emma emma;
    tigress.BuildHits(event);
    emma.BuildHits(event);
    tigress.UpdateBGOFire(Tigress::SUPPRESSION_WINDOW_NS);

    for(const auto& current : tigress.Hits()) {
      Histogramer::Fill("summary",70,0,70,current.ArrayNumber(),
                        8000,0,4000,current.Energy());
    }

    const bool isGood = !emma.Anode().empty() && std::isfinite(emma.PGACX());
    for(const auto& current : tigress.Hits()) {
      Histogramer::Fill("Emma_Tig","summary",70,0,70,
                        current.ArrayNumber(),8000,0,4000,current.Energy());
      if(isGood) {
        Histogramer::Fill("Emma_Tig","summary_good",70,0,70,
                          current.ArrayNumber(),8000,0,4000,current.Energy());
      }
    }

    Histogramer::Fill("Emma_Tig","Si Size",100,0,100,emma.Si().size());
    Histogramer::Fill("Emma_Tig","Anode Size",100,0,100,emma.Anode().size());
    Histogramer::Fill("Emma_Tig","IC0 Size",100,0,100,emma.IC0().size());
    Histogramer::Fill("Emma_Tig","IC1 Size",100,0,100,emma.IC1().size());
    Histogramer::Fill("Emma_Tig","IC2 Size",100,0,100,emma.IC2().size());
    Histogramer::Fill("Emma_Tig","IC3 Size",100,0,100,emma.IC3().size());
    if(!emma.Si().empty()) {
      Histogramer::Fill("Emma_Tig/Si_triggered", "Anode Size", 10,0,10,emma.Anode().size());
    }

    const char *eventType = event.TimestampNs() >= 0 ? "prompt" : "delay";
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": Core.size",100,0,100,tigress.Hits().size());
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": Si.size",100,0,100,emma.Si().size());
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": IC0.size",100,0,100,emma.IC0().size());
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": IC1.size",100,0,100,emma.IC1().size());
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": IC2.size",100,0,100,emma.IC2().size());
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": IC3.size",100,0,100,emma.IC3().size());
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": pgac_l.size",100,0,100,emma.Left().size());
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": pgac_r.size",100,0,100,emma.Right().size());
    Histogramer::Fill("EventProcessing",std::string(eventType) + ": anode.size",100,0,100,emma.Anode().size());

    OutputManager::Get()->FillPhysics(emma, tigress);
    fPushed++;
  }
}
