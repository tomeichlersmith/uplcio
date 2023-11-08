import uplcio
import awkward as ak

def test():
    expected = ak.Array({
        'one': [1.1, 2.2, 3.3],
        'two': [[1],[1,2],[1,2,3]]
    })
    received = uplcio.create_demo_array()

    assert expected.fields == received.fields
    assert ak.all(expected['one'] == received['one'])
    assert ak.all(expected['two'] == received['two'])

