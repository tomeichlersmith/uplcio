import uplcio

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
