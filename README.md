captdriver
====
Libre Driver for CAPT-based Canon Printers
---

*Captdriver* is a driver for Canon printers that only accept the 
proprietary *Canon Advanced Printing Technology* (CAPT) data 
stream format. This includes a significant part of, but not 
the entire Canon laser printer lineup with a model number with 
the LBP- prefix. These printers have been sold under the *i-Sensys*
or *LASER SHOT* brands. [Some other Canon lasers from the D- or 
imageRUNNER series are reported to use CAPT as well][ibm].

At time of writing, the only known way to use these printers is to
install proprietary drivers from Canon.

This driver is an onging work in progress, based on the 
reverse-engineeering efforts of Rildo Pragana, Nicholas Boichat, 
Alexey Galakhov and Benoit Bolsee. It is currently in an early 
alpha stage and is unreliable. Use at your own risk.

An easy way of installing this driver is yet to be devised.
In the meantime, please take a look at the `INSTALL.git` and 
`INSTALL` files for ideas.

Details of the CAPT format, including printer control
protocols, status registers and compression algorithms are
documented in the `SPECS` file. While incomplete and expected to
contain errors and omissions, help with expanding it and making
corrections is greatly appreciated. Please also check the 
repository's wiki for more information.

# Message from Alexey Galakhov about installation:
> 'The file "rastertocapt" has to be installed into CUPS manually.
> If you don't know how to do that, this driver is not for you. :)
> Automatic installation will not be done until the driver reaches
> beta stage to avoid misuse.'

[ibm]: https://www-01.ibm.com/support/docview.wss?uid=nas8N1019527 "IBM. IBM Information on Printers by Canon. IBM Support. 
Reference #N1019527. Updated 2017-03-28."
