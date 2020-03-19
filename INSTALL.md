# Building and Installing captdriver

This software must be manually installed. The steps are fundamentally
as follows:

1. Clone, download and extract the captdriver source to a designated
   directory.

2. Generate the required build files.

3. Run the generated configuration script `./configure` and initiate
   the build process with `make`.

4. Install the driver on your operating system. The build process
   produces a single binary named `rastertocapt` which must either:
   
   i. be copied to both the `filter/` and `driver/` directories under
      the CUPS directory, OR
   
   ii. be copied to either directory, and linked from the other under
       the same name, `rastertocapt`.

5. Configure your Desktop Environment to use the printer. This step
   is not required if you are not using a GUI on the system hosting
   the printer.

Detailed instructions are not included in this document, as the
exact procedure will vary between operating systems.

For detailed instructions, please see Mounaiban's captdriver Wiki
at https://github.com/mounaiban/captdriver/wiki

[wiki]: http://github.com/mounaiban/captdriver/wiki "Mounaiban's captdriver Project Wiki."
