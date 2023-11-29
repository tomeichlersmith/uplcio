Load LCIO files directly into in-memory awkward arrays.

# Abandoned

I got pretty far with implementing this as a proof-of-concept but I reached an issue that I am unable to rectify (detail below). Hopefully this half-finished project will be helpful for someone else in the future.

## Core Issue: LCRelations
A major part of the LCIO format is "relations" between objects. In some cases, this means one object is another object's parent ("constiuent" relations, e.g. a track is related to the hits that make it) or could mean "extra" information that may not always be needed ("LCRelation", e.g. a track fit may yield data about the kinks in the track which would only be necessary if a user wishes to study how the track fit is doing). Relationships like these break the tree-structure of an awkward array, but we can still implement them by using indices and implementing special behaviors (or manually changing the form) - this was discussed in [a scikit-hep/awkward discussion](https://github.com/scikit-hep/awkward/discussions/2838). The core issue is that LCIO implements relations like these _not_ as indices but as pointers in C++. This pointer-based implementation works well on the C++ side, but suffers when trying to translate it to another format in memory. This leaves me with a few options.

1. On each event during translation, construct a map of all `LCObject*` pointers in the event to their collection name and index in that collection. Make this mapping available to all `ReadOnlyBranch`es so that they can translate any constiuent relations or `LCRelation`s that may come up into a look-up ID that can be used on the `awkward` side in python. This has the benefit of maintaining the structure of the LCIO data (as best we can) but has a significant overhead cost for each event.
2. Abandon translation of references from LCIO and adopt an ownership-only model. Constiuent relations are stored directly as members of the owning object and `LCRelation`s copy the `to` object into the `from` object. This avoids this cost of constructing a look-up map, but leads to a confusing relationship between the LCIO data and the in-memory `awkward` data.

Neither of these "solutions" are very satisfying to me and so I'm abandoning this project at this stage. Perhaps a person with more knowledge about the LCIO format or its code base will be able to offer a nicer solution to this problem, but for now I am not working on it.


## Roadmap
The on-disk bit format is not well documented, I literally can't find it anywhere, so I think that means we will have to fall back to using the LCIO C++ library itself in a backend python module. This isn't a large loss since Awkward has a header-only distribution of "builders" that can be put into a python module via pybind11.

Roughly in order, may not be tackled in order if other roadblocks or new ideas arise.
- [x] set up two-tier package structure a la awkward

### _Assuming system install of LCIO_
Basic operations that show ability to parse file and construct a regular array.

- [x] read event headers written by simjob
- [x] read run headers written by simjob
- [x] read collection names (and their types)
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
- [x] select events by index range
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
