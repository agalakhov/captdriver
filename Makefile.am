SUBDIRS = src tests

dist_doc_DATA = SPECS INSTALL.git
EXTRA_DIST = src/canon-lbp.drv ppd
nodist_data_DATA = Makefile.in

ppd: src/canon-lbp.drv
	LC_ALL=C ppdc src/canon-lbp.drv -d ./ppd

clean-ppd:
	-rm -rf ppd
