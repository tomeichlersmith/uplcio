Load LCIO files directly into in-memory awkward arrays.

The on-disk bit format is not well documented, I literally can't find it anywhere, so I think that means we will have to fall back to using the LCIO C++ library itself in a backend python module. This isn't a large loss since Awkward has a header-only distribution of "builders" that can be put into a python module via pybind11.

## Resources

- [iLCSoft/LCIO](https://github.com/ilCSoft/lcio)
    - [dumpevent](https://github.com/iLCSoft/LCIO/blob/master/src/cpp/src/EXAMPLE/dumpevent.cc) source and [dumpEventDetailed](https://github.com/iLCSoft/LCIO/blob/93aff553188450715410bf541066afa3f0a6dbb0/src/cpp/src/UTIL/LCTOOLS.cc#L83) for doing something to all collections in an event
    - [CheckCollections](https://github.com/iLCSoft/LCIO/blob/master/src/cpp/src/UTIL/CheckCollections.cc) for making sure all events have the same collections (putting in empty collections for missing collections)
- [example pybind11 awkward creator](https://github.com/scikit-hep/awkward/tree/main/header-only/examples/pybind11)

## Outline

Generally, I'm thinking we will have two python packages similar to how awkward itself is structured: an underlying package consisting of bound C++ for the heavy-duty functions and an interface package wrapping these strict/complicated functions with easier to use, more flexible, and type-checking ones.

### `uplcio_cpp`

The pybind11-C++ underlying package.

`ReadOnlyFile`: C++ class we bind to Python to read LCIO files, below are its public member functions that are bound as well

- `ReadOnlyFile`: constructor
    - allocate LCIO reader, open input file, and load the names of the collections from the file
    - args:
        - `std::string filepath` location of LCIO file to read
        - `bool first_or_all` choose if we should load collections from all events (true) or only from the first event (false)
- `~ReadOnlyFile`: destructor
    - make sure to close LCIO file and de-allocate the reader

- `get_collections`: get all the collections names
    - return: `std::vector<std::string>` -> `List[str]`
    - args
        -  `bool first_or_all` choose to how retrieve collections
            - `true` would be just get them from the first event, faster but potentially incorrect, while `false` would be explicitly scan the event headers of all events to make sure we get all of the collections
- `load_collections`: actually load data into memory
    - return: `py::object` -> `ak.Array`
    - args
        - `std::vector<std::string> to_load` list of collection names to load
        - `int n_skip` number of events to skip before starting to read and create array
        - `int max_read` maximum number of events to read before returning early, can be `-1` to signal that we should read all the events
        - `bool none_is_empty` silently set any missing collections to the empty collection, if `false` raise an exception if a missing collection is found
- `get_num_events`: get the number of events in the file
    - return: `int`
    - no args
- `get_num_runs`: get the number of runs in the file
    - return: `int`
    - no args
- `get_runs`: load the run headers into memory
    - return: `py::object` -> `ak.Array`
    - args
        - maybe some selection args?

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
