# Building and Installing captdriver

This software must be manually installed. The steps are fundamentally
as follows:

1. Clone, download and extract the captdriver source to a designated
   directory.

2. Generate the required build files.

3. Run the generated configuration script `./configure` and initiate
   the build process with `make`.

4. Install the driver on your operating system. The build process
   produces a single binary named `rastertocapt`, which must be 
   copied to the `filter/` directoy the CUPS directory.

   * *PROTIP*: Reveal the CUPS directory with `cups-config --serverbin`

   * *PROTIP*: For security reasons, CUPS will only run filters that
     are locked down: owned by the `root` user with read-and-execute-only
     permissions to all other users. See this message on
     [Issue #7](https://github.com/agalakhov/captdriver/issues/7#issuecomment-604221791) for details.

5. Compile the PPD file, to introduce your printer to the CUPS 
   daemon.

6. Configure the CUPS daemon to use you printer. There are several
   ways to achieve this.

Detailed instructions are not included in this document, as the
exact procedure will vary between operating systems, and depending
on the approach which you take to install the driver.

For detailed instructions, please see Mounaiban's captdriver Wiki
at https://github.com/mounaiban/captdriver/wiki

[wiki]: http://github.com/mounaiban/captdriver/wiki "Mounaiban's captdriver Project Wiki."
