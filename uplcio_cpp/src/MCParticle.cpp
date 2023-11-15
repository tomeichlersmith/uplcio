#include "ReadOnlyBranch.h"
#include "snapshot.h"
#include "layout_builder_common.h"

#include <EVENT/MCParticle.h>

class MCParticle : public ReadOnlyBranch {
  enum Field : std::size_t {
    id
  };
 RecordBuilder<
   RecordField<Field::id, ListOffsetBuilder<int64_t,NumpyBuilder<long unsigned int>>>
  > builder_;
  static std::map<std::size_t,std::string> field_names;
 public:
  MCParticle() : ReadOnlyBranch(), builder_{field_names} {}
  virtual ~MCParticle() = default;
  virtual pybind11::object snapshot() final {
    return snapshot_builder(builder_);
  }
  virtual void append(lcio::LCCollection* collection) final {
    auto& id{builder_.content<Field::id>().begin_list()};
    if (collection == nullptr) {
      builder_.content<Field::id>().end_list();
      return;
    }
    for (std::size_t i{0}; i < collection->getNumberOfElements(); i++) {
      EVENT::MCParticle* particle{dynamic_cast<EVENT::MCParticle*>(collection->getElementAt(i))};
      id.append(particle->id());
    }
    builder_.content<Field::id>().end_list();
  }
};

std::map<std::size_t, std::string> MCParticle::field_names = {
  {Field::id, "id"}
};

BRANCH_TYPE(MCParticle)
