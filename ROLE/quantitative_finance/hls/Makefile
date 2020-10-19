# *****************************************************************************
# *                            cloudFPGA
# *            All rights reserved -- Property of IBM
# *----------------------------------------------------------------------------
# * Created : October 2018
# * Authors : FAB, WEI, NGL, DID
# *
# * Description : A makefile that invokes all the HLS synthesis batches located
# *   in the current sub-directories.
# *
# ******************************************************************************

.PHONY: all clean mem_test_flash mceuropeanengine

all: mem_test_flash mceuropeanengine


*:
	@#echo ---- Building [ $@ ] ----------------------------
	@$(MAKE) -C $@

clean:
	rm -rf ./*/*_prj
