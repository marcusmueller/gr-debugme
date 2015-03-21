Using GDB to debug GNU Radio applications
-----------------------------------------

**state**: This tutorial was designed for GNU Radio 3.7; if you're using 3.6.5/3.8: this should all work for you, too, but names and directory structures might differ. If your GNU Radio version is older < 3.6.5 and you're trying to understand what your own code does, please update GNU Radio. It doesn't make sense to use outdated versions for development. If you need to debug old software, this might work for you too, but you also probably have GCC/GDB where things might not work as smoothly. Some package names will not be correct for all distributions. Use imagination at own discretion.

**audience**: People whose programs crash. You should feel at home with a shell and comfortable with the process of building your own module. It helps if you roughly know what a compiler does.

Introduction
============

*gdb* stands for *GNU Debugger*. It's the debugger that comes with the GCC (*GNU compiler collection*) set of compilers, and is the preferred choice when debugging native applications that were built using `gcc` (GNU C compiler) or `g++` (GNU C++ compiler).

GDB is a really handy tool, but it can't do magic -- a computer uses numerical handles (basically, addresses) to reference functions, and unless you enrich your library/binary with the symbols that these stand for, GDB will not be able to tell you in which function something went wrong. For your own programs, you can set the build type to `RelWithDebInfo`, which will generate a release type executable (i.e. including all the bugs you only notice when optimization does its thing), but containing Debugging Infos:

```
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

That will also include references to the original source code within the binary -- which will come in really really helpful later on.

For software that you've installed using your distribution's package management (e.g. `yum`, `apt-get`), there's often separate debug info available in form of a package; in Debian-based distros (Debian, Ubuntu, ...) these packages are often `libwhatever-dbg`, for Redhatoids (RHEL, Fedora, CentOS, ...) you get these doing `debuginfo-install libwhatever`.

For debugging GNU Radio programs, I'd recommend building GNU Radio with Debug Infos, and installing the python and boost debuginfo packages, as well as those for `libstdc++`; on Fedora, the latter amounts to a whole 1.3GB of extracted debug infos, but afterwards, debugging something like misuse of `std::vector` or `boost::shared_pointer` gets a lot easier.

A typical problem - Segmentation faults
=======================================

Assume you have a block, which you use in a GNU Radio flowgraph, but it keeps *segfaulting* (i.e. aborting with a *segmentation fault*). This means that *something* in your program tries to access memory that the program is not allowed to access.

Now, you can't really ask the program about its state once it has died; the moment it tries to access the "forbidden" memory, the operating system will handle the resulting CPU exception, and the process has to (and will be) killed, and can't interact with you anymore.

Getting a core dump
___________________

Luckily, there's a feature called *core dumping* that allows the state of your program to be stored in a file, allowing later analysis. Usually, that feature is disabled; you can enable it by:

```
ulimit -c unlimited
```

Note that this only works for processes spawned from the shell that you used `ulimit` in. What happens here is that the maximum size of a core dump is set to *unlimited* (the original value is 0 in most cases).

After that, start your GNU Radio application from there:

```
marcus> ulimit -c unlimited
marcus> ctest -V
<much output>
2: Test command: /usr/bin/sh "/home/marcus/src/gr-debugme/build/python/qa_buggy_cplusplus_sink_test.sh"
2: Test timeout computed to be: 9.99988e+06
2: /home/marcus/src/gr-debugme/build/python/qa_buggy_cplusplus_sink_test.sh: line 8: 22608 Segmentation fault      (core dumped) /usr/bin/python2 /home/marcus/src/gr-debugme/python/qa_buggy_cplusplus_sink.py
2/2 Test #2: qa_buggy_cplusplus_sink ..........***Failed    0.82 sec
```

Finding that core dump
######################

Now, the core dump file lays in the current execution directory of the program that crashed. In our case, that's `build/python/`, but since all core dumps should have a name like `core.<pid>`, we can use a little find magic:

```
marcus> find -type f -cmin 5 -name 'core.[0-9]*'
./build/python/core.22608
```

because that will find all *f*iles, changed/created within the last *5 min*utes, having a name that matches.

Using GDB with a core dump
__________________________

having found `build/python/core.22608`,
we can now launch GDB:

```
gdb programname coredump
```

i.e.

```
gdb /usr/bin/python2 build/python/core.22608
```

A lot of information might scroll by.

At the end, you're greeted by the GDB prompt:

```
(gdb) 
```

Getting a backtrace
###################

Typically, you'd just get a `backtrace` (or shorter, `bt`). A backtrace is simply the hierarchy of functions that were called.

```
(gdb) bt
#0  gr::debugme::buggy_cplusplus_sink_impl::get_address (this=0x139d020, addr=0) at /home/marcus/src/gr-debugme/lib/buggy_cplusplus_sink_impl.cc:49
#1  0x00007fbd080ef8b5 in _wrap_buggy_cplusplus_sink_sptr_get_address (args=<optimized out>, kwargs=<optimized out>)
    at /home/marcus/src/gr-debugme/build/swig/debugme_swigPYTHON_wrap.cxx:5288
#2  0x00007fbd1c610a4e in call_function (oparg=<optimized out>, pp_stack=0x7fffa2a0aac0) at /usr/src/debug/Python-2.7.8/Python/ceval.c:4110
#3  PyEval_EvalFrameEx  [....]
```

Each line contains a stack frame (numbered), the program counter, and the symbol (function name) including the library, or, if that info is available, the source code incl. line number, where that was taken from. Awesome!

Remember, the core was dumped right after the program tried to access memory that did not belong to the program's memory; so the current point of execution is right were we did that; this means that stack frame `#0` points us right at the function that did that. In this case, it's `get_address`, a function that just takes an address and returns the value there as a python-compatible `int`.

We can get the source code at the current point of executio in our current stack frame (`#0` by default) using the `list` command:

```
(gdb) list
44	              gr::io_signature::make(0, 0, 0))
45	    {}
46	    int /* python-friendly type */
47	    buggy_cplusplus_sink_impl::get_address(size_t addr)
48	    {
49	        return (*((uint8_t*)addr))+0;
50	    }
51	
52	    /*
53	     * Our virtual destructor.
```

Notice how line 49, which was where we are in stack frame `#0`, is in the middle. We see that the program does nothing else but give us what is at `addr`. That's horrible if you think about why someone should need something like that, but it's not *why* the program is misbehaving -- something else is calling our function with the wrong `addr`.

Luckily, our backtrace already contains that value: `#0  gr::debugme::buggy_cplusplus_sink_impl::get_address (this=0x139d020, addr=0)` , and since 0 definitelyis not a proper address, we should now look at the caller that did that.

The function that called is `_wrap_buggy_cplusplus_sink_sptr_get_address` and is in a file that is in a `swig` subdirectory. SWIG is a wrapper generator, used by GNU Radio to export the C++ classes to Python and to be able to use Python blocks in C++. SWIG at times is really horrible, it's not free from bugs, but we've worked around a lot of these, and SWIG still sees regular updates. You should, unless you are convinced otherwise, assume that the problem is not within SWIG. So we just skip frame `#1`.

Frame `#2` and following definitely look like they're part of the Python implementation -- that sounds bad, because GDB doesn't itself know how to debug python, but luckily, there's an extension to do that. So we can try to use `py-bt`:

```
(gdb) py-bt
```

If we get a `undefined command` error, we must stop here and make sure that the python development package is installed (`python-devel` on Redhatoids, `python2.7-dev` on Debianoids); for some systems, you should append the content of `/usr/share/doc/{python-devel,python2.7-dev}/gdbinit[.gz]` to your `~/.gdbinit`, and re-start `gdb`.

The output of `py-bt` now states clearly which python lines correspond to which stack frame (skipping those stack frames that are hidden to python, because they are in external libraries or python-implementation routines):

```
(gdb) py-bt
#3 Frame 0x7fbd09ac8938, for file /home/marcus/src/gr-debugme/build/swig/debugme_swig.py, line 198, in get_address (self=<buggy_cplusplus_sink_sptr(this=<SwigPyObject at remote 0x7fbd089b9f30>) at remote 0x7fbd08340d10>, addr=0)
    return _debugme_swig.buggy_cplusplus_sink_sptr_get_address(self, addr)
#6 Frame 0x7fbd0834c730, for file /home/marcus/src/gr-debugme/python/qa_buggy_cplusplus_sink.py, line 38, in test_001_getaddr_t (self=<qa_buggy_cplusplus_sink(src=<vector_source_f_sptr(this=<SwigPyObject at remote 0x7fbd089b9ea0>) at remote 0x7fbd08340c90>, head=<head_sptr(this=<SwigPyObject at remote 0x7fbd089b9ed0>) at remote 0x7fbd08340cd0>, _testMethodName='test_001_getaddr_t', _resultForDoCleanups=<_XMLTestResult(_mirrorOutput=False, _testRunEntered=True, _tests=[], shouldStop=False, _original_stdout=<file at remote 0x7fbd1cafe150>, errors=[], _original_stderr=<file at remote 0x7fbd1cafe1e0>, expectedFailures=[], skipped=[], testsRun=1, buffer=False, _stdout_buffer=None, _test_name='unittest.suite.TestSuite', _start_time=<float at remote 0x14609a8>, _previousTestClass=<type at remote 0x16a7840>, failures=[], _moduleSetUpFailed=False, _error=None, _stderr_buffer=None, unexpectedSuccesses=[], _failure=None, failfast=False) at remote 0x7fbd08340a10>, _cleanups=[], _type_equality_funcs={<type at remote 0x7fbd1c...(truncated)
    self.sink.get_address(0)
#9 Frame 0x13978e0, for file /usr/lib64/python2.7/unittest/case.py, line 367, in run (self=<qa_buggy_cplusplus_sink(src=<vector_source_f_sptr(this=<SwigPyObject at remote 0x7fbd089b9ea0>) at remote 0x7fbd08340c90>, head=<head_sptr(this=<SwigPyObject at remote 0x7fbd089b9ed0>) at remote 0x7fbd08340cd0>, _testMethodName='test_001_getaddr_t', _resultForDoCleanups=<_XMLTestResult(_mirrorOutput=False, _testRunEntered=True, _tests=[], shouldStop=False, _original_stdout=<file at remote 0x7fbd1cafe150>, errors=[], _original_stderr=<file at remote 0x7fbd1cafe1e0>, expectedFailures=[], skipped=[], testsRun=1, buffer=False, _stdout_buffer=None, _test_name='unittest.suite.TestSuite', _start_time=<float at remote 0x14609a8>, _previousTestClass=<type at remote 0x16a7840>, failures=[], _moduleSetUpFailed=False, _error=None, _stderr_buffer=None, unexpectedSuccesses=[], _failure=None, failfast=False) at remote 0x7fbd08340a10>, _cleanups=[], _type_equality_funcs={<type at remote 0x7fbd1c8c3e00>: 'assertSetEqual', <type at remote ...(truncated)
    testMethod()
#14 Frame 0x7fbd089fdd00, for file /usr/lib64/python2.7/unittest/case.py, line 431, in __call__ (self=<qa_buggy_cplusplus_sink(src=<vector_source_f_sptr(this=<SwigPyObject at remote 0x7fbd089b9ea0>) at remote 0x7fbd08340c90>, head=<head_sptr(this=<SwigPyObject at remote 0x7fbd089b9ed0>) at remote 0x7fbd08340cd0>, _testMethodName='test_001_getaddr_t', _resultForDoCleanups=<_XMLTestResult(_mirrorOutput=False, _testRunEntered=True, _tests=[], shouldStop=False, _original_stdout=<file at remote 0x7fbd1cafe150>, errors=[], _original_stderr=<file at remote 0x7fbd1cafe1e0>, expectedFailures=[], skipped=[], testsRun=1, buffer=False, _stdout_buffer=None, _test_name='unittest.suite.TestSuite', _start_time=<float at remote 0x14609a8>, _previousTestClass=<type at remote 0x16a7840>, failures=[], _moduleSetUpFailed=False, _error=None, _stderr_buffer=None, unexpectedSuccesses=[], _failure=None, failfast=False) at remote 0x7fbd08340a10>, _cleanups=[], _type_equality_funcs={<type at remote 0x7fbd1c8c3e00>: 'assertSetEqual', <type ...(truncated)
    return self.run(*args, **kwds)
[...]
```

You can find the module you can do your own crash tests in the "github repo":https://github.com/marcusmueller/gr-debugme.
