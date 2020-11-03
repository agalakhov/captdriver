CUPS driver for CAPT printers
=============================

This is a driver for Canon CAPT-based printers (LBP-****)
based on several reverse engineering attempts. This is
currently in an early alpha stage. Use at your own risk.

The printer protocol documentation based on reverse engineering
is in the SPECS file. This is incomplete and probably has many
errors. Please help expanding it.

Supported printers
------------------

  * Canon LBP-2900
  * Canon LBP-3000

Build
-----

The project uses GNU Autotools to build the driver.
Run the following commands after cloning the repo:

```
$ aclocal
$ autoconf
$ automake --add-missing
```

Install
-------

The file "rastertocapt" has to be installed into CUPS manually.
If you don't know how to do that, this driver is not for you. :)
Automatic installation will not be done until the driver reaches
beta stage to avoid misuse.
