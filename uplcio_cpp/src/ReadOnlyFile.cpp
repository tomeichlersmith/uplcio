#include "ReadOnlyFile.h"
#include "ReadOnlyBranch.h"

#include <EVENT/LCEvent.h>

/**
 * A simple lock-like class which just makes sure we close
 * the reader if we ever go out of scope. This helps make sure that
 * the reader is closed when leaving a function (either successfully
 * or through an exception).
 */
class OpenedFile {
  IO::LCReader& reader_;
 public:
  OpenedFile(IO::LCReader& reader, const std::string& filepath)
    : reader_{reader} {
      reader_.open(filepath);
  }
  ~OpenedFile() {
    reader_.close();
  }
};

ReadOnlyFile::ReadOnlyFile(
    const std::string& filepath,
    bool use_only_first_event_for_collections
): filepath_{filepath}, reader_{lcio::LCFactory::getInstance()->createLCReader()} {
  /**
   * We might want to use a non-default set of LCReader flags
   * to better optimize our reading style.
   *
   * LCReader is an abstract class which is implemented by
   * SIOReader which itself uses MT::LCReader as its concrete
   * file reader, so looking there for the implementation details.
   *
   * https://github.com/iLCSoft/LCIO/blob/93aff553188450715410bf541066afa3f0a6dbb0/src/cpp/src/MT/LCReader.cc#L32-L37
   */
  collections_ = get_collections(use_only_first_event_for_collections, true);
  OpenedFile op(*reader_, filepath_);
  num_events_ = reader_->getNumberOfEvents();
  num_runs_ = reader_->getNumberOfRuns();
}

ReadOnlyFile::~ReadOnlyFile() {}

const std::unordered_map<std::string,std::string>& ReadOnlyFile::get_collections(
    bool use_only_first_event_for_collections,
    bool reread
) {
  if (collections_.empty() and not reread) 
    return collections_;

  auto get_collections_from_event = [&](const EVENT::LCEvent* event) {
    /**
     * While LCEvent::getCollectionNames technically returns a pointer,
     * it is actually returning the address of one of its member variables,
     * so we skip any validity check.
     */
    const auto* collection_names = event->getCollectionNames();
    for (const auto& name : *collection_names) {
      if (collections_.find(name) == collections_.end()) {
        // new collection, need to load it so we can get its type string
        auto collection = event->getCollection(name);
        collections_[name] = collection->getTypeName();
      }
    }
    return;
  };

  collections_.clear();
  // LCIO just keeps event header information as apart of the LCEvent
  // and not listed under the getCollectionNames so we manually put it in
  collections_["EventHeader"] = "EventHeader";
  OpenedFile op(*reader_, filepath_);
  if (use_only_first_event_for_collections) {
    const auto event = reader_->readNextEvent();
    get_collections_from_event(event);
    return collections_;
  } 

  while(const auto event = reader_->readNextEvent()) {
    get_collections_from_event(event);
  }
  return collections_;
}

int ReadOnlyFile::get_num_events() {
  return num_events_;
}

int ReadOnlyFile::get_num_runs() {
  return num_runs_;
}

pybind11::object ReadOnlyFile::load_collections(
    const std::vector<std::string>& to_load,
    int n_skip,
    int max_read,
    bool none_is_empty
) {
  std::map<std::string,std::unique_ptr<ReadOnlyBranch>> branches;
  for (const std::string& load : to_load) {
    if (collections_.find(load) == collections_.end()) {
      throw std::runtime_error("Requested collection '"+load+"' is not in collection listing");
    }
    branches.emplace(load, ReadOnlyBranch::Factory::get().create(collections_[load]));
  }
  OpenedFile op(*reader_, filepath_);
  if (n_skip > 0) {
    reader_->skipNEvents(n_skip);
  }
  int event_count{0};
  lcio::LCEvent* event{0};
  if (max_read < 0) max_read = get_num_events()+1;
  while((event = reader_->readNextEvent()) != 0 and event_count++ < max_read) {
    for (auto& [name, branch] : branches) {
      branch->append(event, name);
    }
  }
  std::map<std::string,pybind11::object> converted_branches;
  for (auto& [name, branch] : branches) {
    converted_branches[name] = branch->snapshot();
  }
  auto zip = pybind11::module::import("awkward").attr("zip");
  return zip(converted_branches);
}

pybind11::dict ReadOnlyFile::load_runs() {
  OpenedFile op(*reader_, filepath_);
  EVENT::LCRunHeader* run_header{0};
  std::vector<long int> number;
  std::vector<std::string> detector_name;
  std::vector<std::string> description;
  std::vector<std::vector<std::string>> active_subdetectors;
  while((run_header = reader_->readNextRunHeader()) != 0) {
    number.push_back(run_header->getRunNumber());
    detector_name.push_back(run_header->getDetectorName());
    description.push_back(run_header->getDescription());
    auto asds{run_header->getActiveSubdetectors()};
    active_subdetectors.emplace_back();
    if (asds != nullptr) {
      active_subdetectors.back().reserve(asds->size()); 
      for (const std::string& sd : *asds) {
        active_subdetectors.back().push_back(sd);
      }
    }
  }
  return pybind11::dict(
      pybind11::arg("number") = number,
      pybind11::arg("detector_name") = detector_name,
      pybind11::arg("description") = description,
      pybind11::arg("active_subdetectors") = active_subdetectors
  );
}
