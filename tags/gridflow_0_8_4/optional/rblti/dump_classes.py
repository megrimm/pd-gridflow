
import inspect

import pylti as lti


members = inspect.getmembers(lti)


for e in members:
    name = e[0]
    if not name.endswith('Ptr'):
        print name #,type(e)
