import uplcio
import awkward as ak
import numpy as np

def test_open(simjob_filepath):
    f = uplcio.ReadOnlyFile(simjob_filepath)


def test_numbers(simjob_filepath):
    """The simjob.slcio file is created by the simjob executable
    example shipped with an LCIO installation. This writes 10 runs
    into the file each with 10 events."""

    f = uplcio.ReadOnlyFile(simjob_filepath)
    assert f.num_events == 100
    assert f.num_runs == 10


def test_event_header(simjob_filepath):
    f = uplcio.ReadOnlyFile(simjob_filepath)
    headers = f.load_collections(['EventHeader'])
    assert ak.all(headers.EventHeader.number == np.concatenate(10*[np.arange(0,10,1)]))
    assert ak.all(headers.EventHeader.run == np.concatenate([np.full(10,run) for run in range(10)]))


def test_partial_reads(simjob_filepath):
    f = uplcio.ReadOnlyFile(simjob_filepath)
    # read first 10
    headers = f.load_collections(['EventHeader'],max_read=10)
    assert ak.all(headers.EventHeader.number == np.arange(0,10,1))
    assert ak.all(headers.EventHeader.run == 0)

    #read another 10
    headers = f.load_collections(['EventHeader'],n_skip=10, max_read=10)
    assert ak.all(headers.EventHeader.number == np.arange(0,10,1))
    assert ak.all(headers.EventHeader.run == 1)


def test_run_header(simjob_filepath):
    f = uplcio.ReadOnlyFile(simjob_filepath)
    runs = ak.Array(f.load_runs())
    assert ak.all(runs.number == np.arange(0,10,1))
    assert ak.all(runs.detector_name == 'D09TileHcal')
