from pathlib import Path
from uplcio import from_lcio
events = from_lcio('test.slcio')
print(events, len(events))
print(repr(events.type))
