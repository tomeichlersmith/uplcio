#include "ReadOnlyFile.h"
#include "ReadOnlyBranch.h"

#include <EVENT/LCEvent.h>

ReadOnlyFile::ReadOnlyFile(
    const std::string& filepath,
    bool use_only_first_event_for_collections
): reader_{lcio::LCFactory::getInstance()->createLCReader()} {
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
  reader_->open(filepath);
  collections_ = get_collections(use_only_first_event_for_collections, true);
}

ReadOnlyFile::~ReadOnlyFile() {
  reader_->close();
}

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
  return reader_->getNumberOfEvents();
}

int ReadOnlyFile::get_num_runs() {
  return reader_->getNumberOfRuns();
}

pybind11::object ReadOnlyFile::load_collections(
    const std::vector<std::string>& to_load,
    int n_skip,
    int max_read,
    bool none_is_empty
) {
  if (n_skip > 0) {
#ifdef DEBUG
    std::cout << "skipping " << n_skip << " events" << std::endl;
#endif
    reader_->skipNEvents(n_skip);
  }
  int event_count{0};
  lcio::LCEvent* event{0};
#ifdef DEBUG
  std::cout << "branch init" << std::endl;
#endif
  std::map<std::string,std::unique_ptr<ReadOnlyBranch>> branches;
  branches.emplace("header", ReadOnlyBranch::Factory::get().create("EventHeader"));
  branches.emplace("MCParticle", ReadOnlyBranch::Factory::get().create(EVENT::LCIO::MCPARTICLE));
#ifdef DEBUG
  std::cout << "begin reading branches" << std::endl;
#endif
  while((event = reader_->readNextEvent()) != 0 and event_count++ < max_read) {
    for (auto& [name, branch] : branches) {
      branch->append(event, name);
    }
  }
#ifdef DEBUG
  std::cout << "done reading branches" << std::endl;
#endif
  // TODO: do we need to reset the file state somehow?
  // in order to allow re-reading without re-opening (e.g. getting a few more branches)
  // we would need to update the state of the file
#ifdef DEBUG
  std::cout << "converting branch builders to ak.Arrays" << std::endl;
#endif
  std::map<std::string,pybind11::object> converted_branches;
  for (auto& [name, branch] : branches) {
#ifdef DEBUG
    std::cout << "  " << name << std::endl;
#endif
    converted_branches[name] = branch->snapshot();
  }
#ifdef DEBUG
  std::cout << "zipping branches together" << std::endl;
#endif
  auto zip = pybind11::module::import("awkward").attr("zip");
  return zip(converted_branches);
}

pybind11::object ReadOnlyFile::load_runs() {
  return {};
}
