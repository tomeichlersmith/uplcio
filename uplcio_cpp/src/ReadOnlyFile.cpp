#include "ReadOnlyFile.h"

ReadOnlyFile::ReadOnlyFile(
    const std::string& filepath,
    bool use_only_first_event_for_collections
) {
  /**
   * This factory just creates an SIOReader and up-casts to
   * its abstract base class LCReader upon return.
   */
  reader_ = lcio::LCFactory::getInstance()->createLCReader(); 
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
  collections_ = get_collections(use_only_first_event_for_collections);
}

ReadOnlyFile::~ReadOnlyFile() {
  reader_->close();
}

std::vector<std::string> ReadOnlyFile::get_collections(
    bool use_only_first_event_for_collections
) {
  return {};
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
  return {};
}

pybind11::object ReadOnlyFile::load_runs() {
  return {};
}