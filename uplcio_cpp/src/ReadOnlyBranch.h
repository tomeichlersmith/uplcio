#pragma once

#include <memory>

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
  virtual void append(lcio::LCEvent* event, const std::string& collection_name) {
    append(event->getCollection(collection_name));
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
