from pathlib import Path
from demo import from_lcio
events = from_lcio('test.slcio')
print(events, len(events))
print(repr(events.type))
