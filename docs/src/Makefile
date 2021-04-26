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
DOXYGEN_XML_TARGET_DIR = xml-cpp-docs


# doxyrest settings
DOXYREST_FRAME = $(dir $(shell which doxyrest))/../share/doxyrest/frame
DOXYREST_RST_TARGET_DIR = rst-cpp-docs

.PHONY: all install xml rst html clean cleanhtml cleanall 

all: install

install: .stamp/sphinx
	rsync -av --delete --exclude .git --exclude .buildinfo \
		 _build/html/ $(HTML_DEST_DIR) 1>/dev/null


#######################################################################################################################

#
# Cosine Similarity Alveo Product
#

COSINESIM_RST_NAMES = api.rst cpp-api.rst cpp-classes.rst demos.rst index.rst installation.rst
COSINESIM_RST_FILES = $(addprefix cosinesim/,$(COSINESIM_RST_NAMES))
COSINESIM_VERSION = ../../cosinesim/VERSION
COSINESIM_TRIGGER_FILES = ../../cosinesim/include/cosinesim.hpp

.stamp/cosinesim-doxygen: .stamp cosinesim/$(DOXYGEN_CPP_CONFIG_FILE) $(COSINESIM_TRIGGER_FILES)
	cd cosinesim && \
	rm -rf $(DOXYGEN_XML_TARGET_DIR) && \
	mkdir -p $(DOXYGEN_XML_TARGET_DIR) && \
	sed 's,COSINESIM_VERSION,$(strip $(shell cat $(COSINESIM_VERSION))),g' < $(DOXYGEN_CPP_CONFIG_FILE) \
	    > $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	doxygen $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	rm -f $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	cd .. && \
	touch .stamp/cosinesim-doxygen

.stamp/cosinesim-doxyrest: .stamp/cosinesim-doxygen cosinesim/doxyrest-config.lua
	cd cosinesim && \
	rm -rf $(DOXYREST_RST_TARGET_DIR) && \
	mkdir -p $(DOXYREST_RST_TARGET_DIR) && \
	doxyrest $(DOXYGEN_XML_TARGET_DIR)/index.xml -o $(DOXYREST_RST_TARGET_DIR)/index.rst && \
	rm -f $(DOXYREST_RST_TARGET_DIR)/index.rst && \
	cd .. && \
	touch .stamp/cosinesim-doxyrest

# Above recipe removes index.rst, as it is replaced by a hand-generated one, and this one causes a warning

.PHONY: cosinesim-clean

cosinesim-clean:
	cd cosinesim; \
	rm -rf $(DOXYGEN_XML_TARGET_DIR) $(DOXYREST_RST_TARGET_DIR)

#######################################################################################################################

RST_FILES = index.rst overview.rst recomengine-example-jupyter.rst \
            recomengine-example-script.rst cosinesim-example-makefile.rst \
            cosinesim-example-jupyter.rst cosinesim-example-python.rst \
            $(COSINESIM_RST_FILES)

.PHONY: doxyrest

doxyrest: .stamp .stamp/cosinesim-doxyrest

.stamp/sphinx: doxyrest Makefile-sphinx.mk $(RST_FILES) conf.py
	make -f Makefile-sphinx.mk html && \
	touch .stamp/sphinx

html: cleanhtml .stamp/sphinx

.stamp:
	mkdir -p .stamp


clean: cosinesim-clean cleanhtml
	rm -rf .stamp

cleanhtml:
	rm -rf .stamp/sphinx _build/html _build/doctrees

