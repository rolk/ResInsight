#  Copyright (C) 2017 Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

import importlib
import os
import sys
import traceback
import unittest
import inspect

class ImportTestCase(unittest.TestCase):
    
    
    def import_module(self , module):
        mod = importlib.import_module( module )
        return mod
            
    def import_package(self, package):
        module = self.import_module( package )
        path = os.path.dirname( inspect.getfile( module ) )

        for entry in sorted(os.listdir(path)):
            entry_path = os.path.join(path, entry)
            if os.path.isdir( entry_path ):
                module = os.path.basename( entry )
                sub_module = "%s.%s" % (package , module)
                self.import_package( sub_module )
            else:
                module, ext = os.path.splitext( entry )
                if module == "__init__":
                    continue

                if ext == "py":
                    self.import_module("%s.%s" % (package , module))

        return True
