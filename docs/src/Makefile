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

ifeq (, $(shell which doxygen 2>/dev/null))
$(error "No doxygen in PATH.")
endif
ifeq (, $(shell which doxyrest 2>/dev/null))
$(error "No doxyrest in PATH.")
endif
ifeq (, $(shell which sphinx-build 2>/dev/null))
$(error "No sphinx-build in PATH.")
endif
ifeq (, $(HTML_DEST_DIR))
$(error "HTML_DEST_DIR is not set.")
endif

# doxygen settings
DOXYGEN_CPP_CONFIG_FILE = DoxyFile-cpp
COSINESIM_VERSION = ../../cosinesim/VERSION
DOXYGEN_XML_TARGET_DIR = xml-cpp-docs

# doxyrest settings
DOXYREST_FRAME = $(dir $(shell which doxyrest))/../share/doxyrest/frame
DOXYREST_RST_TARGET_DIR = rst-cpp-docs
RST_FILES = index.rst overview.rst api-doc.rst 

.PHONY: all install xml rst html clean cleanhtml cleanall 

all: install

install: .stamp/sphinx
	rsync -av --delete --exclude .git --exclude .buildinfo \
		 _build/html/ $(HTML_DEST_DIR) 1>/dev/null

.stamp/doxygen: $(DOXYGEN_CPP_CONFIG_FILE)
	rm -rf $(DOXYGEN_XML_TARGET_DIR) && \
	mkdir $(DOXYGEN_XML_TARGET_DIR) && \
	sed 's,COSINESIM_VERSION,$(strip $(shell cat $(COSINESIM_VERSION))),g' < $(DOXYGEN_CPP_CONFIG_FILE) > $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	doxygen $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	rm -f $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	mkdir -p .stamp && touch .stamp/doxygen

.stamp/doxyrest: .stamp/doxygen doxyrest-config.lua
	rm -rf $(DOXYREST_RST_TARGET_DIR) && \
	mkdir $(DOXYREST_RST_TARGET_DIR) && \
	doxyrest $(DOXYGEN_XML_TARGET_DIR)/index.xml -o $(DOXYREST_RST_TARGET_DIR)/index.rst && \
	touch .stamp/doxyrest

.stamp/sphinx: .stamp/doxyrest Makefile-sphinx.mk $(RST_FILES) conf.py
	mkdir -p .stamp && \
	make -f Makefile-sphinx.mk html && \
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
	rm -rf .stamp _build/html _build/doctrees