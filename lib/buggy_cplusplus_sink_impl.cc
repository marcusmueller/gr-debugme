/* -*- c++ -*- */
/* 
 * Copyright 2015 Marcus Müller.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "buggy_cplusplus_sink_impl.h"

namespace gr {
  namespace debugme {

    buggy_cplusplus_sink::sptr
    buggy_cplusplus_sink::make()
    {
      return gnuradio::get_initial_sptr
        (new buggy_cplusplus_sink_impl());
    }

    /*
     * The private constructor
     */
    buggy_cplusplus_sink_impl::buggy_cplusplus_sink_impl()
      : gr::sync_block("buggy_cplusplus_sink",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(0, 0, 0))
    {}
    int /* python-friendly type */
    buggy_cplusplus_sink_impl::get_address(size_t addr)
    {
        return (*((uint8_t*)addr))+0;
    }

    /*
     * Our virtual destructor.
     */
    buggy_cplusplus_sink_impl::~buggy_cplusplus_sink_impl()
    {
    }

    int
    buggy_cplusplus_sink_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];

        double sum = 0.0;
        // let's have a logical bug: typo "+=" should be "<=", and basically should be <, and basically you could also boost::foreach this
        for(int counter = 0; counter += noutput_items -1; ++counter)
          // and now, segfault away!
          sum += in[counter];

        std::cout << sum << std::endl;

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace debugme */
} /* namespace gr */

