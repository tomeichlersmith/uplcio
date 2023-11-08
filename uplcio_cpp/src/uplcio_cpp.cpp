// BSD 3-Clause License; see https://github.com/scikit-hep/awkward/blob/main/LICENSE

#include "demo.h"

PYBIND11_MODULE(uplcio_cpp, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("create_demo_array", &create_demo_array, "A function that creates an awkward array");
}
