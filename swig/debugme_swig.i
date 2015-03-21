/* -*- c++ -*- */

#define DEBUGME_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "debugme_swig_doc.i"

%{
#include "debugme/buggy_cplusplus_sink.h"
%}


%include "debugme/buggy_cplusplus_sink.h"
GR_SWIG_BLOCK_MAGIC2(debugme, buggy_cplusplus_sink);
