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


#ifndef INCLUDED_DEBUGME_BUGGY_CPLUSPLUS_SINK_H
#define INCLUDED_DEBUGME_BUGGY_CPLUSPLUS_SINK_H

#include <debugme/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace debugme {

    /*!
     * \brief Sink that should segfault in work()
     * \ingroup debugme
     *
     */
    class DEBUGME_API buggy_cplusplus_sink : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<buggy_cplusplus_sink> sptr;

      /*!
       * \brief gives you the byte at any address.
       * Hi. I'm an open invitation for segfaults.
       */
      virtual int get_address(size_t addr) = 0;

      /*!
       * \brief Return a shared_ptr to a new instance of debugme::buggy_cplusplus_sink.
       *
       * To avoid accidental use of raw pointers, debugme::buggy_cplusplus_sink's
       * constructor is in a private implementation
       * class. debugme::buggy_cplusplus_sink::make is the public interface for
       * creating new instances.
       */
      static sptr make();
    };

  } // namespace debugme
} // namespace gr

#endif /* INCLUDED_DEBUGME_BUGGY_CPLUSPLUS_SINK_H */

