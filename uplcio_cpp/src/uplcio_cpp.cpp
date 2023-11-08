// BSD 3-Clause License; see https://github.com/scikit-hep/awkward/blob/main/LICENSE

#include "demo.h"
#include "ReadOnlyFile.h"

namespace py = pybind11;

PYBIND11_MODULE(uplcio_cpp, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("create_demo_array", &create_demo_array, "A function that creates an awkward array");

    py::class_<ReadOnlyFile>(m, "ReadOnlyFile")
      .def(py::init<const std::string&, bool>())
      .def("get_collections", &ReadOnlyFile::get_collections)
      .def("get_num_events", &ReadOnlyFile::get_num_events)
      .def("get_num_runs", &ReadOnlyFile::get_num_runs)
      .def("load_collections", &ReadOnlyFile::load_collections)
      .def("load_runs", &ReadOnlyFile::load_runs)
    ;
}
