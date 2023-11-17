from ._cpp import ReadOnlyFile
from uplcio_cpp import create_demo_array

import awkward as ak

@ak.mixin_class(ak.behavior)
class MCParticle:
    def parents(self):
        return self['_parent_indices']
