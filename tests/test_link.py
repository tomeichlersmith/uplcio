import uplcio
import awkward as ak
import numpy as np

def test_open(simjob_filepath):
    print(simjob_filepath)
    f = uplcio.ReadOnlyFile(simjob_filepath,False)

def test_numbers(simjob_filepath):
    """The simjob.slcio file is created by the simjob executable
    example shipped with an LCIO installation. This writes 10 runs
    into the file each with 10 events."""

    f = uplcio.ReadOnlyFile(simjob_filepath, False)
    assert f.get_num_events() == 100
    assert f.get_num_runs() == 10

def test_event_header(simjob_filepath):
    f = uplcio.ReadOnlyFile(simjob_filepath, False)
    headers = f.load_collections([],0,50,False)
    assert ak.all(headers.header.number == np.concatenate(5*[np.arange(0,10,1)]))
    assert ak.all(headers.header.run == np.concatenate([np.full(10,run) for run in range(5)]))
