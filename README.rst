libucd
======

What is this?
-------------

libucd is a C++ library that provides a simple and efficient way to access
properties from the Unicode Character Database from your code.  The goal is to
have an independent library that anyone can use to obtain access to the
Unicode Character Database without having to extract the data themselves,
while providing access to all of the defined properties.

It’s written using features of C++11, so it won’t work on older compilers.
Sorry.

Licensing
---------

See LICENSING.  I’d appreciate it if you could at least mention me in your
documentation somewhere if you use this library in your code.

Building libucd
---------------

You will need scons <http://www.scons.org>, and a C++11 compiler.  To build
the library, just enter ``scons`` at your shell prompt.  To run the tests, use
``scons check``.

Using libucd
------------

There are examples of every function in the ``tests`` folder, but to get you
started, the first thing you need to do is to open a Unicode Database file::

  ucd::database db;

  db.open('/path/to/unicode-x.y.z.ucd');

These files are built from the raw UCD data using the ``ucdc`` tool, which you
can find in the ``tools`` folder.

Once you have the file open, you can query the database, e.g.::

  if (db.general_category(cp) != General_Category::Nd) {
    std::cerr << "I expected a digit" << std::endl;
  }
