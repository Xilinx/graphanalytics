#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

.PHONY: all install xml rst html clean cleanhtml cleanall check_tools

all: install

check_tools:
ifeq (, $(shell which sphinx-build 2>/dev/null))
	@echo "ERROR: No sphinx-build in PATH" && false
endif

install: .stamp/sphinx
ifeq (, $(HTML_DEST_DIR))
	@echo "ERROR: HTML_DEST_DIR is not set." && false
endif
	rsync -av --delete --exclude .git --exclude .nojekyll --exclude .buildinfo \
		 _build/html/ $(HTML_DEST_DIR) 1>/dev/null

.stamp/sphinx: Makefile-sphinx.mk | check_tools
	make -f Makefile-sphinx.mk html
	touch .stamp/sphinx

html:
	mkdir -p .stamp
	rm -rf .stamp/sphinx _build/doctrees _build/html
	make .stamp/sphinx

clean:
	rm -rf .stamp

cleanhtml:
	rm -rf .stamp/sphinx _build/html _build/doctrees

cleanall:
	rm -rf .stamp xml xml_2 xml_3 rst rst_2 rst_3 _build/html _build/doctrees