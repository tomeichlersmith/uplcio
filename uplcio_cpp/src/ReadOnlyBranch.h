#pragma once

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <lcio.h>
#include <EVENT/LCEvent.h>
#include <EVENT/LCCollection.h>

#include "awkward/LayoutBuilder.h"

using UserDefinedMap = std::map<std::size_t, std::string>;
template<class... BUILDERS>
using RecordBuilder = awkward::LayoutBuilder::Record<UserDefinedMap, BUILDERS...>;
template<std::size_t field_name, class BUILDER>
using RecordField = awkward::LayoutBuilder::Field<field_name, BUILDER>;
template<class PRIMITIVE, class BUILDER>
using ListOffsetBuilder = awkward::LayoutBuilder::ListOffset<PRIMITIVE, BUILDER>;
template<class PRIMITIVE>
using NumpyBuilder = awkward::LayoutBuilder::Numpy<PRIMITIVE>;

/**
 * Abstract class so we can hold all our branches in one container while reading
 */
class ReadOnlyBranch {
 protected:
  /**
   * Create a snapshot of the given builder, and return an `ak.Array` pyobject
   * @tparam T type of builder
   * @param builder builder
   * @return pyobject of Awkward Array
   */
  template<typename T>
  static pybind11::object snapshot_builder(const T &builder) {
      // How much memory to allocate?
      std::map <std::string, size_t> names_nbytes = {};
      builder.buffer_nbytes(names_nbytes);
  
      // Allocate memory
      std::map<std::string, void *> buffers = {};
      for (auto it: names_nbytes) {
          uint8_t *ptr = new uint8_t[it.second];
          buffers[it.first] = (void *) ptr;
      }
  
      // Write non-contiguous contents to memory
      builder.to_buffers(buffers);
      static auto from_buffers = pybind11::module::import("awkward").attr("from_buffers");
  
      // Build Python dictionary containing arrays
      // dtypes not important here as long as they match the underlying buffer
      // as Awkward Array calls `frombuffer` to convert to the correct type
      pybind11::dict container;
      for (auto it: buffers) {
  
          pybind11::capsule free_when_done(it.second, [](void *data) {
              uint8_t *dataPtr = reinterpret_cast<uint8_t *>(data);
              delete[] dataPtr;
          });
  
          uint8_t *data = reinterpret_cast<uint8_t *>(it.second);
          container[pybind11::str(it.first)] = pybind11::array_t<uint8_t>(
                  {names_nbytes[it.first]},
                  {sizeof(uint8_t)},
                  data,
                  free_when_done
          );
      }
      return from_buffers(builder.form(), builder.length(), container);
  }
 public:
  virtual ~ReadOnlyBranch() = default;
  virtual void append(lcio::LCCollection* collection) = 0;
  virtual pybind11::object snapshot() = 0;
  virtual void append(lcio::LCEvent* event, const std::string& collection_name) {
    try {
      append(event->getCollection(collection_name));
    } catch (const lcio::DataNotAvailableException&) {
      // TODO: implement method to pass user's choice on how to handle this case
      // now, I am doing what I want which is just an empty collection
      append(nullptr);
    }
  }
  class Factory {
    template<class BranchType>
    static std::unique_ptr<ReadOnlyBranch> maker() {
      return std::make_unique<BranchType>();
    }
    std::map<std::string, std::unique_ptr<ReadOnlyBranch>(*)()> library_;
    Factory() = default;
   public:
    static Factory& get() {
      static Factory instance_;
      return instance_;
    }
    std::unique_ptr<ReadOnlyBranch> create(const std::string& name) {
      auto lib_it{library_.find(name)};
      if (lib_it == library_.end()) {
        throw std::runtime_error("Object of type "+name+" has not been declared");
      }
      return lib_it->second();
    }
    template<class BranchType>
    uint64_t declare(const std::string& name) {
      auto lib_it{library_.find(name)};
      if (lib_it != library_.end()) {
        throw std::runtime_error("An object named " + name +
            " has already been declared.");
      }
      library_[name] = &maker<BranchType>;
      return reinterpret_cast<std::uintptr_t>(&library_);
    }
  };
};

#define BRANCH_TYPE(CLASS,NAME) \
  namespace { \
    auto v = ::ReadOnlyBranch::Factory::get().declare<CLASS>(NAME); \
  }
