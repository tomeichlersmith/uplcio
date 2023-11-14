#include "ReadOnlyBranch.h"

#include "EventHeader.h"

void ReadOnlyBranch::append(lcio::LCEvent* event, const std::string& collection_name) {
  append(event->getCollection(collection_name)); 
}

std::unique_ptr<ReadOnlyBranch> ReadOnlyBranch::create(const std::string& type_name) {
  if (type_name == EventHeader::TYPE_NAME) {
    return std::make_unique<EventHeader>();
  } else {
    throw std::runtime_error("Unrecognized type "+type_name);
  }
}
