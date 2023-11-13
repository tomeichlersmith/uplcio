#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "awkward/LayoutBuilder.h"

#include <lcio.h>
#include <IO/LCReader.h>
#include <IOIMPL/LCFactory.h>
#include <EVENT/LCEvent.h>
#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

namespace py = pybind11;

/**
 * Create a snapshot of the given builder, and return an `ak.Array` pyobject
 *
 * This is taken directly from awkward's documentation.
 *
 * @tparam T type of builder
 * @param builder builder
 * @return pyobject of Awkward Array
 */
template<typename T>
py::object snapshot_builder(const T &builder) {
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
    auto from_buffers = py::module::import("awkward").attr("from_buffers");

    // Build Python dictionary containing arrays
    // dtypes not important here as long as they match the underlying buffer
    // as Awkward Array calls `frombuffer` to convert to the correct type
    py::dict container;
    for (auto it: buffers) {

        py::capsule free_when_done(it.second, [](void *data) {
            uint8_t *dataPtr = reinterpret_cast<uint8_t *>(data);
            delete[] dataPtr;
        });

        uint8_t *data = reinterpret_cast<uint8_t *>(it.second);
        container[py::str(it.first)] = py::array_t<uint8_t>(
                {names_nbytes[it.first]},
                {sizeof(uint8_t)},
                data,
                free_when_done
        );
    }
    return from_buffers(builder.form(), builder.length(), container);

}

// Defines taken from how-to-use-header-only-layoutbuilder.md
using UserDefinedMap = std::map<std::size_t, std::string>;
template<class... BUILDERS>
using RecordBuilder = awkward::LayoutBuilder::Record<UserDefinedMap, BUILDERS...>;
template<std::size_t field_name, class BUILDER>
using RecordField = awkward::LayoutBuilder::Field<field_name, BUILDER>;
template<class PRIMITIVE, class BUILDER>
using ListOffsetBuilder = awkward::LayoutBuilder::ListOffset<PRIMITIVE, BUILDER>;
template<class PRIMITIVE>
using NumpyBuilder = awkward::LayoutBuilder::Numpy<PRIMITIVE>;

class Branch {
 public:
  virtual ~Branch() = default;
  virtual void append(lcio::LCCollection* coll) = 0;
  virtual void append(lcio::LCEvent* evt, const std::string& name) {
    append(evt->getCollection(name));
  }
  virtual py::object snapshot() = 0;
};

class EventHeader : public Branch {
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
  EventHeader(): builder_{field_names} {}
  void append(lcio::LCCollection*) final override {}
  void append(lcio::LCEvent* evt, const std::string& name) final override {
    builder_.field<Field::number>().append(evt->getEventNumber());
    builder_.field<Field::run>().append(evt->getRunNumber());
    builder_.field<Field::timestamp>().append(evt->getTimeStamp());
    builder_.field<Field::weight>().append(evt->getWeight());
  }
  py::object snapshot() final override {
    return snapshot_builder(builder_);
  }
};
std::map<std::size_t,std::string> EventHeader::field_names = {
  {Field::number, "number"},
  {Field::run, "run"},
  {Field::timestamp, "timestamp"},
  {Field::weight, "weight"}
};

class MCParticle : public Branch {
  enum Field : std::size_t {
    id
  };
  RecordBuilder<
    RecordField<Field::id, ListOffsetBuilder<std::size_t, NumpyBuilder<long unsigned int>>>
  > builder_;
  static std::map<std::size_t, std::string> field_names;
 public:
  MCParticle(): builder_{field_names} {}
  void append(lcio::LCCollection* coll) final override {
    auto &list = builder_.field<Field::id>().begin_list();
    for (std::size_t i{0}; i < coll->getNumberOfElements(); i++) {
      EVENT::MCParticle* particle{dynamic_cast<EVENT::MCParticle*>(coll->getElementAt(i))};
      list.append(particle->id());
    }
    builder_.field<Field::id>().end_list();
  }
  py::object snapshot() final override {
    return snapshot_builder(builder_);
  }
};
std::map<std::size_t,std::string> MCParticle::field_names = {
  {Field::id, "id"}
};


/**
 * can we open an lcio file?
 */
py::object from_lcio(const std::string& f) {
  std::cout << "opening file " << f << std::endl;
  std::cout << " attempt to create reader..." << std::endl;
  IO::LCReader* lc_reader_{IOIMPL::LCFactory::getInstance()->createLCReader()}; 
  std::cout << " opening file" << std::endl;
  lc_reader_->open(f);
  lcio::LCEvent* evt{0};
  std::size_t nevents{0};
  std::map<std::string, std::unique_ptr<Branch>> branches;
  branches.emplace("header", std::make_unique<EventHeader>());
  if ((evt = lc_reader_->readNextEvent()) != 0) {
    const std::vector<std::string>* collections = evt->getCollectionNames();
    std::cout << collections->size() << std::endl;
    for (const std::string& name : *collections) {
      lcio::LCCollection* collection = evt->getCollection(name);
      if (collection->getTypeName() == EVENT::LCIO::MCPARTICLE) {
        branches.emplace(name, std::make_unique<MCParticle>());
        branches[name]->append(collection);
      } else {
        std::cerr << "WARN: " << name << " of type " << collection->getTypeName() << " not implemented."
          << std::endl;
      }
    }
  } else {
    throw std::runtime_error("Unable to read a single event from lcio file.");
  }
  while((evt = lc_reader_->readNextEvent()) != 0 and nevents++ < 10000) {
    branches["header"]->append(evt, "");
    for (const std::string& name : *(evt->getCollectionNames())) {
      if (branches.find(name) != branches.end()) {
        branches[name]->append(evt, name);
      }
    }
  }
  std::cout << " closing file" << std::endl;
  lc_reader_->close();
  std::cout << "leaving" << std::endl;
  std::cout << "converting builders to ak.Arrays" << std::endl;
  std::map<std::string,py::object> conv_branches;
  for (auto it = branches.begin(); it != branches.end(); ++it) {
    std::cout << "  " << it->first << std::endl;
    conv_branches[it->first] = it->second->snapshot();
  }
  auto zip = py::module::import("awkward").attr("zip");
  std::cout << "zipping up" << std::endl;
  return zip(conv_branches);
}

PYBIND11_MODULE(uplcio, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("from_lcio", &from_lcio, "can we open an lcio file?");
}
