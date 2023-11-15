#include "ReadOnlyBranch.h"
#include "snapshot.h"
#include "layout_builder_common.h"

class EventHeader : public ReadOnlyBranch {
  enum Field : std::size_t {
    number,
    run,
    timestamp,
    weight
  };
  RecordBuilder<
    RecordField<Field::number, NumpyBuilder<long unsigned int>>,
    RecordField<Field::run, NumpyBuilder<long unsigned int>>,
    RecordField<Field::timestamp, NumpyBuilder<long unsigned int>>,
    RecordField<Field::weight, NumpyBuilder<double>>
  > builder_;
  static std::map<std::size_t,std::string> field_names;
 public:
  EventHeader() : ReadOnlyBranch(), builder_{field_names} {}
  virtual ~EventHeader() = default;
  virtual void append(lcio::LCCollection* collection) final {}
  virtual void append(lcio::LCEvent* event, const std::string& name) final {
    builder_.content<Field::number>().append(event->getEventNumber());
    builder_.content<Field::run>().append(event->getRunNumber());
    builder_.content<Field::timestamp>().append(event->getTimeStamp());
    builder_.content<Field::weight>().append(event->getWeight());
  }
  virtual pybind11::object snapshot() final {
    return snapshot_builder(builder_);
  }
};

std::map<std::size_t,std::string> EventHeader::field_names = {
  {Field::number, "number"},
  {Field::run, "run"},
  {Field::timestamp, "timestamp"},
  {Field::weight, "weight"}
};

BRANCH_TYPE(EventHeader,"EventHeader")
