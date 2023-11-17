#include "ReadOnlyBranch.h"
#include "snapshot.h"
#include "layout_builder_common.h"

#include <EVENT/MCParticle.h>

class MCParticle : public ReadOnlyBranch {
  enum Field : std::size_t {
    energy,
    pdg,
    _parent_indices,
    _daughter_indices
  };
  ListOffsetBuilder<int64_t,
    RecordBuilder<
      RecordField<Field::energy, NumpyBuilder<double>>,
      RecordField<Field::pdg, NumpyBuilder<int>>,
      RecordField<Field::_parent_indices, ListOffsetBuilder<int64_t,NumpyBuilder<unsigned int>>>,
      RecordField<Field::_daughter_indices, ListOffsetBuilder<int64_t, NumpyBuilder<unsigned int>>>
    >
  > builder_;
  static std::map<std::size_t,std::string> field_names;
 public:
  MCParticle() : ReadOnlyBranch(), builder_{} { builder_.content().set_fields(field_names); }
  virtual ~MCParticle() = default;
  virtual pybind11::object snapshot() final {
    return snapshot_builder(builder_);
  }
  virtual void append(lcio::LCCollection* collection) final {
    auto& particles{builder_.begin_list()};
    if (collection == nullptr) {
      builder_.end_list();
      return;
    }
    // need to loop over particles twice, once to map pointers to collection indices
    // and then again to actually create and fill the particles
    std::map<EVENT::MCParticle*, unsigned int> particle_index_lut;
    for (unsigned int i{0}; i < collection->getNumberOfElements(); i++) {
      EVENT::MCParticle* particle{dynamic_cast<EVENT::MCParticle*>(collection->getElementAt(i))};
      particle_index_lut[particle] = i;
    }
    for (std::size_t i{0}; i < collection->getNumberOfElements(); i++) {
      EVENT::MCParticle* particle{dynamic_cast<EVENT::MCParticle*>(collection->getElementAt(i))};
      particles.content<Field::energy>().append(particle->getEnergy());
      particles.content<Field::pdg>().append(particle->getPDG());

      std::vector<unsigned int> parents;
      parents.reserve(particle->getParents().size());
      for (EVENT::MCParticle* parent : particle->getParents()) {
        parents.push_back(particle_index_lut[parent]);
      }
      particles.content<Field::_parent_indices>().begin_list().extend(parents.data(), parents.size());
      particles.content<Field::_parent_indices>().end_list();
      std::vector<unsigned int> daughters;
      daughters.reserve(particle->getDaughters().size());
      for (EVENT::MCParticle* daughter : particle->getDaughters()) {
        daughters.push_back(particle_index_lut[daughter]);
      }
      particles.content<Field::_daughter_indices>().begin_list().extend(daughters.data(), daughters.size());
      particles.content<Field::_daughter_indices>().end_list();
    }
    builder_.end_list();
  }
};

std::map<std::size_t, std::string> MCParticle::field_names = {
  {Field::energy, "energy"},
  {Field::pdg, "pdg"},
  {Field::_parent_indices, "_parent_indices"},
  {Field::_daughter_indices, "_daughter_indices"}
};

BRANCH_TYPE(MCParticle, EVENT::LCIO::MCPARTICLE)
