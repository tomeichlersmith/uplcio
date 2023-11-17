"""Wrap the bound C++ with some Python syntax sugar"""

import uplcio_cpp

class ReadOnlyFile(uplcio_cpp.ReadOnlyFile):
    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_val, exc_tb):
        del self
