# Building and Installing captdriver

## Summary
This software must be manually installed. The steps are fundamentally
as follows:

1. Clone, download and extract the captdriver source to a designated
   directory.

2. Generate the required build files.

3. Run the generated configuration script `./configure` and initiate
   the build process with `make`.

4. Install the driver on your operating system.

5. Configure your Desktop Environment to use the printer. This step
   is not required if you are not using a GUI on the system hosting
   the printer.

## Prerequisites
Please ensure that you have the build dependencies installed on your
system.

On Ubuntu, this includes all packages of `build-essential`,
`automake` and `libcups2-dev`. Installing `git` is completely
optional.

Command to ensure dependencies on Ubuntu 18.04.1 are installed: 

`sudo apt install build-essential git automake libcups2-dev`

Guides for building on more systems will be made available in the
[project Wiki][wiki].

_Please do also check if CUPS is correctly set up on your system_ 😸

## Cloning
Please skip this step if you prefer to download the ZIP file and
extract it manually on your system.

Use this command: `git clone https://github.com/mounaiban/captdriver`
in a directory where you would like to keep source files.

> If you prefer, clone Galakhov's original instead: `git clone https://github.com/`

## Building
If you've downloaded the source code from Git, you will find that 
running the usual CMMI^ routine doesn't work, due to missing
files. Run these commands in sequence to generate the necessary files,
and build the driver:

```
# Create the configure script
aclocal
autoconf
automake --add-missing
# Build the driver and install (the CMMI routine)
./configure
make
sudo make install
```

^ `./confgure; make; make install;`

## Installation
The above steps builds a single binary, `rastertocapt` and places it
in the `/usr/local/bin` directory. However, the driver is not yet
ready to use.

To begin using the driver, copy the `rastertocapt` driver to the CUPS
directories, and set up the required links by running the following
commands in sequence:

```
sudo cp -p /usr/local/bin/rastertocapt $(cups-config --serverbin)/driver/
ln -sf ($cups-config --serverbin)/driver/rastertocapt $(cups-config --serverbin)/filter/rastertocapt
```

> *PROTIP*: The `cups-config --serverbin` command expands to the
full path to the directory where CUPS is installed.

> *PROTIP*: The `-p` in `cp- p` preserves timestamps (among other
attributes) across all copies of a file. This makes identical files
less prone to being mistaken for modified copies when listed with
commands like `ls -l`.

> *NOTE*: Preliminary testing has shown that at least one actual
copy of the `rastertocapt` binary must be in the CUPS directories
for print jobs to succeed. Creating links to `/usr/local/bin/rastertocapt`
may result in mysterious print job failures.

The drivers are now installed on your system and ready to use.

But wait, there's more! (I said the _driver_ was ready, not the
printer!)

## Configuring Your Desktop Environment 
To begin using your printer with a GUI desktop environment (DE),
further configuration in your DE is required... 😩

If you have not connected your printer, power it on and connect it to
your system. If you are using any of the major GNU/Linux DEs, the
system should attempt to detect the printer and fail. But don't worry,
you still set the printer up manually.

Once this happens, go to the relevant section in the control panel of
your DE, where you would configure printing devices, and manually add
the printer, choosing to provide a PPD file. When prompted, use the
`Canon-LBP-2900.ppd` file in this repository.

If you have previously set up your printer, you may just need to
reconfigure it to use the PPD file.

> Please note that there is only one PPD file, but it should work with
> the printers tested so far, including the LBP2900 and LBP3000.

Due to differences between DEs, instructions on how to configure your
printer are not included in this document.

However, guides will be posted on the [project Wiki][wiki]

In the meantime, please consult the documentation or user forums for
your DE software.

[wiki]: http://github.com/mounaiban/captdriver/wiki "Mounaiban's captdriver Project Wiki."
