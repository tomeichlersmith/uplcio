// BSD 3-Clause License; see https://github.com/scikit-hep/awkward/blob/main/LICENSE

#include "ReadOnlyFile.h"

namespace py = pybind11;

PYBIND11_MODULE(uplcio_cpp, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    py::class_<ReadOnlyFile>(m, "ReadOnlyFile")
      .def(
          py::init<const std::string&, bool>(),
          py::arg("filepath"),
          py::arg("use_only_first") = false
      )
      .def(
          "get_collections",
          &ReadOnlyFile::get_collections,
          "Retrieve the list of collections (and their types) from within the file",
          py::arg("use_only_first") = false,
          py::arg("reread") = false
      )
      .def_property_readonly(
          "collections",
          [](ReadOnlyFile& self) {
            return self.get_collections();
          }
      )
      .def_property_readonly(
          "num_events",
          &ReadOnlyFile::get_num_events
      )
      .def_property_readonly(
          "num_runs",
          &ReadOnlyFile::get_num_runs
      )
      .def(
          "load_collections",
          &ReadOnlyFile::load_collections,
          "load_collections docstring",
          py::arg("to_load") = std::vector<std::string>{},
          py::arg("n_skip") = 0,
          py::arg("max_read") = -1,
          py::arg("none_is_empty") = true
      )
      .def(
          "load_runs",
          &ReadOnlyFile::load_runs
      )
    ;
}
