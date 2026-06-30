// Added by Claude: Include proper header for edm::stream::EDProducer
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "DataFormats/Common/interface/View.h"

// Added by Claude: Include the ESGetToken header
#include "FWCore/Utilities/interface/ESGetToken.h"

#include <vector>
#include <string>

// Added by Claude: Fixed the class declaration with proper namespace qualification
class HCALHitPositionTableProducer : public edm::stream::EDProducer<> {
public:
  explicit HCALHitPositionTableProducer(const edm::ParameterSet &);
  ~HCALHitPositionTableProducer() override {};

  // Added by Claude: Add static methods required by the CMSSW framework
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  virtual void produce(edm::Event &, const edm::EventSetup &) override;

  edm::EDGetTokenT<HBHERecHitCollection> hcalRecHitsToken_;
  
  // edm::ESHandle<CaloGeometry> caloGeom_;
  // Added by Claude: Declare an ESGetToken for CaloGeometry
  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeomToken_;
};

HCALHitPositionTableProducer::HCALHitPositionTableProducer(const edm::ParameterSet &iConfig) {
  hcalRecHitsToken_ = consumes<HBHERecHitCollection>(iConfig.getParameter<edm::InputTag>("hcalRecHits"));
  
  // Added by Claude: Initialize the ESGetToken in the constructor
  caloGeomToken_ = esConsumes<CaloGeometry, CaloGeometryRecord>();
  
  produces<nanoaod::FlatTable>("HCALHitPositions");
}

// Added by Claude: Implementation of the static method required by the framework
void HCALHitPositionTableProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("hcalRecHits", edm::InputTag("hbhereco"));
  descriptions.addWithDefaultLabel(desc);
}

void HCALHitPositionTableProducer::produce(edm::Event &iEvent, const edm::EventSetup &iSetup) {
  // iSetup.get<CaloGeometryRecord>().get(caloGeom_);
  // Added by Claude: Get the CaloGeometry using the ESGetToken
  const CaloGeometry& caloGeom = iSetup.getData(caloGeomToken_);
  
  edm::Handle<HBHERecHitCollection> hcalRecHitsHandle;
  iEvent.getByToken(hcalRecHitsToken_, hcalRecHitsHandle);

  std::vector<float> eta;
  std::vector<float> phi;
  std::vector<float> energy;
  std::vector<float> time;
  std::vector<int> depth;
  std::vector<int> ieta;
  std::vector<int> iphi;

  for (const auto& hit : *hcalRecHitsHandle) {
    HcalDetId id = hit.id();
    
    // Using the CaloGeometry to get the position
    // Added by Claude: Access CaloGeometry directly instead of through the ESHandle
    auto cell = caloGeom.getPosition(id);
    
    eta.push_back(cell.eta());
    phi.push_back(cell.phi());
    energy.push_back(hit.energy());
    time.push_back(hit.time());
    depth.push_back(id.depth());
    ieta.push_back(id.ieta());
    iphi.push_back(id.iphi());
  }

  auto table = std::make_unique<nanoaod::FlatTable>(eta.size(), "HCALHitPositions", false);
  table->addColumn<float>("eta", eta, "eta", 10);
  table->addColumn<float>("phi", phi, "phi", 10);
  table->addColumn<float>("energy", energy, "energy", 10);
  table->addColumn<float>("time", time, "time", 10);
  table->addColumn<int>("depth", depth, "depth", 10);
  table->addColumn<int>("ieta", ieta, "ieta", 10);
  table->addColumn<int>("iphi", iphi, "iphi", 10);

  iEvent.put(std::move(table), "HCALHitPositions");
}

DEFINE_FWK_MODULE(HCALHitPositionTableProducer);
