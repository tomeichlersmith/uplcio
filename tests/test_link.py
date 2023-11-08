import uplcio

def test_open():
    f = uplcio.ReadOnlyFile("simjob.slcio",False)
    del f
