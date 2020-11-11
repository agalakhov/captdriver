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
Run the following commands after cloning the repo to build the driver:

```
$ aclocal
$ autoconf
$ automake --add-missing
$ ./configure
$ make
```

Install
-------

The file "rastertocapt" has to be installed into CUPS manually.
If you don't know how to do that, this driver is not for you. :)
Automatic installation will not be done until the driver reaches
beta stage to avoid misuse.

Contribute
----------

The driver can potentially support various Canon GDI printers which
have a similar protocol and share and the compression algorithm:

  * LBP-1120
  * LBP-1210
  * LBP-3300
  * LBP-3200
  * LBP-5000
  * LBP-5100
  * LBP-5300

Physical access to a printer is needed to identify the protocol differences
of each individual model. In order to reverse engineer you need the following:

1) Please make yourself familiar with the reverse-engineered LBP2900 protocol provided in the SPECS file
2) You have to find a way of printing with Canon's official driver (presumably under Windows) and intercept USB communication. Any USB sniffer will do. Windows can be installed in a virtual machine.
3) Look at any differences in commands. Try to vary printing parameters and look what changes.
4) Guess the meaning of individual bytes.
5) Submit your findings as pull requests to this repo or open an issue.

There are some helper utilities written by me at early stages of reverse engineering, you can make use of them. They're here: https://github.com/agalakhov/anticapt
Another possibility is using the "captfilter" binary from the official Linux driver. It generates output containing some (but not all) commands needed to print. Try running it with different input parameters and look what it does.