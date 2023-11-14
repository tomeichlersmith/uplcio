#pragma once

#include <pybind11/pybind11.h>

#include <lcio.h>
#include <EVENT/LCEvent.h>
#include <EVENT/LCCollection.h>

/**
 * Abstract class so we can hold all our branches in one container while reading
 */
class ReadOnlyBranch {
 public:
  virtual ~ReadOnlyBranch() = default;
  virtual void append(lcio::LCCollection* collection) = 0;
  virtual pybind11::object snapshot() = 0;
  virtual void append(lcio::LCEvent* event, const std::string& collection_name);
  static std::unique_ptr<ReadOnlyBranch> create(const std::string& type_name);
};
