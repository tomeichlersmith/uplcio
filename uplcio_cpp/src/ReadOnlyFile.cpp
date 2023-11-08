#include "ReadOnlyFile.h"

ReadOnlyFile::ReadOnlyFile(
    const std::string& filepath,
    bool use_only_first_event_for_collections
) {
   reader_ = lcio::LCFactory::getInstance()->createLCReader(); 
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
  return -1;
}

int ReadOnlyFile::get_num_runs() {
  return -1;
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
