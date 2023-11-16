Load LCIO files directly into in-memory awkward arrays.

# Alpha Stage

The on-disk bit format is not well documented, I literally can't find it anywhere, so I think that means we will have to fall back to using the LCIO C++ library itself in a backend python module. This isn't a large loss since Awkward has a header-only distribution of "builders" that can be put into a python module via pybind11.

## Roadmap
Roughly in order, may not be tackled in order if other roadblocks or new ideas arise.
- [x] set up two-tier package structure a la awkward

### _Assuming system install of LCIO_
Basic operations that show ability to parse file and construct a regular array.

- [x] read event headers written by simjob
- [ ] read run headers written by simjob
- [ ] read collection names (and their types)
- [ ] cross-collection referencing behavior in awkward[^1]

[^1]: Necessary for LCRelations as well as other "constituent" relations (like track hits making up a track). The [physlite behaviors](https://github.com/CoffeaTeam/coffea/blob/master/src/coffea/nanoevents/methods/physlite.py) in coffea seem to implement this cross-collection referencing behavior that I would like (this implementation looks like they store a reference to the full event array and its indices and then can return referenced collections from sub-collections by using these references passed around).

### Priority Collection Types
As a first pass, not going to attempt to implement all of the LCIO Event objects.
[The full list](https://ilcsoft.desy.de/LCIO/current/doc/doxygen_api/html/classEVENT_1_1LCObject.html)
is quite long and so I'm going to focus on a core set related to my work on HPS.

- [ ] MCParticle
- [ ] SimTrackerHit
- [ ] SimCalorimeterHit
- [ ] LCGenericObject
- [ ] TrackerHit
- [ ] Track (with optional LCRelations to define additional data?)
- [ ] CalorimeterHit
- [ ] Cluster
- [ ] ReconstructedParticle
- [ ] TrackState

### _Expanding Usability_
- [ ] select events by index range
- [ ] pass `LCIO_DIR` into `uplcio_cpp` build
- [ ] add LCIO as another `FetchContent` option
- [ ] investigate LCIO schema evolution - can one build of `uplcio_cpp` handle multiple LCIO schema versions?
- [ ] read LCRelations and put their pointer behavior into `ak.behavior` somehow

## Resources

- [iLCSoft/LCIO](https://github.com/ilCSoft/lcio)
    - [dumpevent](https://github.com/iLCSoft/LCIO/blob/master/src/cpp/src/EXAMPLE/dumpevent.cc) source and [dumpEventDetailed](https://github.com/iLCSoft/LCIO/blob/93aff553188450715410bf541066afa3f0a6dbb0/src/cpp/src/UTIL/LCTOOLS.cc#L83) for doing something to all collections in an event
    - [CheckCollections](https://github.com/iLCSoft/LCIO/blob/master/src/cpp/src/UTIL/CheckCollections.cc) for making sure all events have the same collections (putting in empty collections for missing collections)
- [example pybind11 awkward creator](https://github.com/scikit-hep/awkward/tree/main/header-only/examples/pybind11)

## Outline

Generally, I'm thinking we will have two python packages similar to how awkward itself is structured: an underlying package consisting of bound C++ for the heavy-duty functions and an interface package wrapping these strict/complicated functions with easier to use, more flexible, and type-checking ones.

### [uplcio_cpp](./uplcio_cpp)

The pybind11-C++ underlying package.

### `uplcio`

The interface package that wraps the bound reader to make it more ergonomic and pythonic. Not going to explicitly outline an API right now, but I do want to note some ideas.

- Context manager semantics for `with`-style interaction with `ReadOnlyFile`
- A central `open` function to handle the different parameters, do argument checking
- Helper `arrays` or `load` implementation that goes straight to reading the objects into memory
    - Optionally load runs into memory (return `events` or `(events, runs)`)
    - Do type checking, and collection existence checking?
    - Some basic filtering by collection name?
- a `lcio2parquet` program using the `ak.to_parquet` function after loading
- optionally cache the `lcio` file with a faster `parquet` file?
