#include "EventHeader.h"

#include "snapshot.h"
#include "layout_builder_common.h"

std::map<std::size_t,std::string> EventHeader::field_names = {
  {Field::number, "number"},
  {Field::run, "run"},
  {Field::timestamp, "timestamp"},
  {Field::weight, "weight"}
};

const std::string EventHeader::TYPE_NAME = "EventHeader";

EventHeader::EventHeader()
  : ReadOnlyBranch(),
    builder_{field_names}
{}

void EventHeader::append(lcio::LCCollection*) {}

void EventHeader::append(lcio::LCEvent* event, const std::string& name) {
  builder_.content<Field::number>().append(event->getEventNumber());
  builder_.content<Field::run>().append(event->getRunNumber());
  builder_.content<Field::timestamp>().append(event->getTimeStamp());
  builder_.content<Field::weight>().append(event->getWeight());
}

pybind11::object EventHeader::snapshot() {
  return snapshot_builder(builder_);
}

