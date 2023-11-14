#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <lcio.h>
#include <IO/LCReader.h>

/**
 * Central wrapper around LCIO to help bind it to python
 */
class ReadOnlyFile {
  /// our reader instance
  std::unique_ptr<lcio::LCReader> reader_;
  /// list of collections in our file
  std::vector<std::string> collections_;
 public:
  /**
   * prepare ourselves to read the input file
   *
   * We allocate an LCIO reader, open the input file, and load the names
   * of the collections from the file.
   *
   * @see get_collections for how we retreive the list of collections from the file
   *
   * @param[in] filepath path to file we should open
   * @param[in] use_only_first_event_for_collections if true, only check first event for collection listing
   */
  ReadOnlyFile(
      const std::string& filepath,
      bool use_only_first_event_for_collections
  );

  /**
   * Close up the file we are reading and de-allocate the LCIO reader
   */
  ~ReadOnlyFile();

  /**
   * Retreive the list of collections within the file.
   *
   * @param[in] use_only_first_event_for_collections if true, only check first event for collection listing
   * @return vector of collection names, newly allocated
   */
  std::vector<std::string> get_collections(
      bool use_only_first_event_for_collections
  );

  /**
   * Get the number of events in the file
   *
   * @return int number of events in the file
   */
  int get_num_events();

  /**
   * Get the number of runs in the file
   *
   * @return int number of runs in the file
   */
  int get_num_runs();

  /**
   * Load the passed collections into in-memory arrays and build
   * them into an awkward array
   *
   * @param[in] to_load names of collections to load into memory
   * @param[in] n_skip number of events at beginning of file to skip
   * @param[in] max_read maximum number of events to read
   * @param[in] none_is_empty silently interpret missing collections as empty
   * @return pybind11::object that is meant to be an ak.Array
   */
  pybind11::object load_collections(
      const std::vector<std::string>& to_load,
      int n_skip,
      int max_read,
      bool none_is_empty
  );

  /**
   * Load the runs into memory and format build them into
   * an awkward array
   *
   * @return pybind11::object that is meant to be an ak.Array
   */
  pybind11::object load_runs();
};
