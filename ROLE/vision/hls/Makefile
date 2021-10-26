# *****************************************************************************
# *                            cloudFPGA
# *            All rights reserved -- Property of IBM
# *----------------------------------------------------------------------------
# * Created : Sep 2018
# * Authors : Francois Abel, Burkhard Ringlein, Dionysios Diamantopoulos
# *
# * Description : A makefile that invokes all the HLS synthesis batches located
# *   in the current sub-directories.
# *
# ******************************************************************************

.PHONY: all clean mem_test_flash median_blur

all: mem_test_flash median_blur


*:
	@#echo ---- Building [ $@ ] ----------------------------
	@$(MAKE) -C $@

clean:
	rm -rf ./*/*_prj
