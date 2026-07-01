#include "PhysicsTools/NanoAOD/interface/SimpleFlatTableProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"

// added by Claude: cms_pepr migration from pepr_15_1_0
// Local exact-type variant of SimpleFlatTableProducer. Reads Handle<std::vector<T>>
// instead of Handle<edm::View<T>> to avoid AmbiguousProduct in pre1 when the
// input module also emits a RefVector<T> under the same tag (e.g. mix:MergedCaloTruth
// produces both vector<SimCluster> and RefVector<SimCluster>).
//
// The base SimpleFlatTableProducerBase<T, C> already supports non-View collection
// types; we just derive from it with C = std::vector<T> and re-implement the
// cut/singleton/maxLen/extvars logic in the same shape as SimpleFlatTableProducer<T>.
template <typename T>
class SimpleFlatTableProducerFromCollection : public SimpleFlatTableProducerBase<T, std::vector<T>> {
public:
  SimpleFlatTableProducerFromCollection(edm::ParameterSet const& params)
      : SimpleFlatTableProducerBase<T, std::vector<T>>(params),
        singleton_(params.getParameter<bool>("singleton")),
        maxLen_(params.existsAs<unsigned int>("maxLen") ? params.getParameter<unsigned int>("maxLen")
                                                        : std::numeric_limits<unsigned int>::max()),
        cut_(!singleton_ ? params.getParameter<std::string>("cut") : "",
             !singleton_ ? params.getUntrackedParameter<bool>("lazyEval") : false) {}

  ~SimpleFlatTableProducerFromCollection() override {}

  std::unique_ptr<nanoaod::FlatTable> fillTable(const edm::Event& iEvent,
                                                 const edm::Handle<std::vector<T>>& prod) const override {
    std::vector<const T*> selobjs;
    if (prod.isValid() || !this->skipNonExistingSrc_) {
      if (singleton_) {
        assert(prod->size() == 1);
        selobjs.push_back(&(*prod)[0]);
      } else {
        for (const auto& obj : *prod) {
          if (cut_(obj)) selobjs.push_back(&obj);
          if (selobjs.size() >= maxLen_) break;
        }
      }
    }
    auto out = std::make_unique<nanoaod::FlatTable>(selobjs.size(), this->name_, singleton_, this->extension_);
    for (const auto& var : this->vars_)
      var->fill(selobjs, *out);
    return out;
  }

protected:
  bool singleton_;
  const unsigned int maxLen_;
  const StringCutObjectSelector<T> cut_;
};
// end added by Claude

// added by Claude: cms_pepr migration from pepr_15_1_0
// SimCluster and CaloParticle share the ambiguity issue (both emitted alongside
// a RefVector by CaloTruthAccumulator under identical tags), so both use the
// exact-collection variant. Other types (PCaloHit, CaloRecHit, CaloCluster) are
// unaffected and keep the View-based producer.
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
typedef SimpleFlatTableProducerFromCollection<SimCluster> SimpleSimClusterFlatTableProducer;

#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
typedef SimpleFlatTableProducerFromCollection<CaloParticle> SimpleCaloParticleFlatTableProducer;
// end added by Claude

#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
typedef SimpleFlatTableProducer<PCaloHit> SimplePCaloHitFlatTableProducer;
#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"
typedef SimpleFlatTableProducer<CaloRecHit> SimpleCaloRecHitFlatTableProducer;

#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
typedef SimpleFlatTableProducer<reco::CaloCluster> SimpleCaloClusterFlatTableProducer;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SimplePCaloHitFlatTableProducer);
DEFINE_FWK_MODULE(SimpleCaloRecHitFlatTableProducer);
DEFINE_FWK_MODULE(SimpleSimClusterFlatTableProducer);
DEFINE_FWK_MODULE(SimpleCaloParticleFlatTableProducer);
DEFINE_FWK_MODULE(SimpleCaloClusterFlatTableProducer);
