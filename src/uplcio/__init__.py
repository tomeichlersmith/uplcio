from ._cpp import ReadOnlyFile

import awkward as ak

@ak.mixin_class(ak.behavior)
class MCParticle:
    def parents(self):
        return self['_parent_indices']
