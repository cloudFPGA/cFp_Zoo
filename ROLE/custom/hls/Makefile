# *****************************************************************************
# *                            cloudFPGA
# *                Copyright 2016 -- 2022 IBM Corporation
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *----------------------------------------------------------------------------
#
# * Created : Sep 2018
# * Authors : Francois Abel, Burkhard Ringlein, Dionysios Diamantopoulos
# *
# * Description : A makefile that invokes all the HLS synthesis batches located
# *   in the current sub-directories.
# *
# ******************************************************************************

.PHONY: all clean mem_test_flash uppercase

all: mem_test_flash uppercase


*:
	@#echo ---- Building [ $@ ] ----------------------------
	@$(MAKE) -C $@

clean:
	rm -rf ./*/*_prj
