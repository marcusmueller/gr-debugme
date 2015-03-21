#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2015 Marcus Müller.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import debugme_swig as debugme

class qa_buggy_cplusplus_sink (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()
        self.src = blocks.vector_source_f([float(i) for i in xrange(10)])
        self.head = blocks.head(gr.sizeof_float, int(1e6))
        self.sink = debugme.buggy_cplusplus_sink()
        self.tb.connect(self.src, self.head, self.sink)

    def tearDown (self):
        self.tb = None
    def test_001_getaddr_t (self):
        self.sink.get_address(0)

        # set up fg
        self.tb.run ()
        # check data


if __name__ == '__main__':
    gr_unittest.run(qa_buggy_cplusplus_sink, "qa_buggy_cplusplus_sink.xml")
