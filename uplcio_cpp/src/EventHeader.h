#pragma once

#include "ReadOnlyBranch.h"
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
  static const std::string TYPE_NAME;
  EventHeader();
  virtual ~EventHeader() = default;
  virtual void append(lcio::LCCollection* collection) final;
  virtual void append(lcio::LCEvent* event, const std::string& name) final;
  virtual pybind11::object snapshot() final;
};
