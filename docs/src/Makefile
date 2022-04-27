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
    $(info "HTML_DEST_DIR not set. Use the default name html-output")
	HTML_DEST_DIR = html-output
endif
HTML_BUILD_DIR = _build/html

# doxygen settings
DOXYGEN_CPP_CONFIG_FILE = DoxyFile-cpp
DOXYGEN_XML_TARGET_DIR = xml-cpp-docs


# doxyrest settings
DOXYREST_FRAME = $(dir $(shell which doxyrest))/../share/doxyrest/frame
DOXYREST_RST_TARGET_DIR = rst-cpp-docs

.PHONY: all install xml rst html clean cleanhtml cleanall 

all: install

install: .stamp/sphinx update-html-versions
	mkdir -p $(HTML_DEST_DIR)
	rsync -av --delete --exclude .git --exclude .buildinfo \
		 $(HTML_BUILD_DIR)/ $(HTML_DEST_DIR) 1>/dev/null


#######################################################################################################################

#
# Cosine Similarity Alveo Product
#

COSINESIM_RST_NAMES = \
    api.rst cpp-api.rst cpp-classes.rst demos.rst index.rst install.rst \
    cosinesim-example-makefile.rst cosinesim-example-jupyter.rst cosinesim-example-python.rst

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

#
# Recommendation Engine for TigerGraph 3.x
#

RECOM_TG3_RST_NAMES = \
    api.rst index.rst install.rst overview.rst \
    synthea-script.rst synthea-jupyter.rst \
    synthea-install.rst synthea-overview.rst synthea-query.rst synthea.rst \
    users-guide.rst

RECOM_TG3_RST_FILES = $(addprefix recom-tg3/,$(RECOM_TG3_RST_NAMES))
RECOM_TG3_SRC_DIR = ../../plugin/tigergraph
RECOM_TG3_VERSION = $(RECOM_TG3_SRC_DIR)/VERSION
RECOM_TG3_TRIGGER_FILES = $(RECOM_TG3_SRC_DIR)/udf/xilinxRecomEngine.hpp

#.stamp/recom-tg3-doxygen: .stamp recom-tg3/$(DOXYGEN_CPP_CONFIG_FILE) $(RECOM_TG3_TRIGGER_FILES)
.stamp/recom-tg3-doxygen: .stamp recom-tg3/$(DOXYGEN_CPP_CONFIG_FILE)
	cd recom-tg3 && \
	rm -rf $(DOXYGEN_XML_TARGET_DIR) && \
	mkdir -p $(DOXYGEN_XML_TARGET_DIR) && \
	sed 's,RECOM_TG3_VERSION,$(strip $(shell cat $(RECOM_TG3_VERSION))),g' < $(DOXYGEN_CPP_CONFIG_FILE) \
	    > $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	doxygen $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	rm -f $(DOXYGEN_XML_TARGET_DIR)/doxyfile.tmp && \
	cd .. && \
	touch .stamp/recom-tg3-doxygen

.stamp/recom-tg3-doxyrest: .stamp/recom-tg3-doxygen recom-tg3/doxyrest-config.lua
	cd recom-tg3 && \
	rm -rf $(DOXYREST_RST_TARGET_DIR) && \
	mkdir -p $(DOXYREST_RST_TARGET_DIR) && \
	doxyrest $(DOXYGEN_XML_TARGET_DIR)/index.xml -o $(DOXYREST_RST_TARGET_DIR)/index.rst && \
	rm -f $(DOXYREST_RST_TARGET_DIR)/index.rst && \
	cd .. && \
	touch .stamp/recom-tg3-doxyrest

# Above recipe removes index.rst, as it is replaced by a hand-generated one, and this one causes a warning

.PHONY: recom-tg3-clean

recom-tg3-clean:
	cd recom-tg3; \
	rm -rf $(DOXYGEN_XML_TARGET_DIR) $(DOXYREST_RST_TARGET_DIR)


#######################################################################################################################
# Product versions
#
# Add product version variables used in rst files to be replaced
#

# Package version
PACKAGE_VERSION=$(strip $(shell cat ../../scripts/amd-graphanalytics-install/VERSION))

# Standalone products
COSINESIM_VERSION=$(strip $(shell cat ../../cosinesim/VERSION))
LOUVAIN_VERSION=$(strip $(shell cat ../../louvainmod/VERSION))
FUZZYMATCH_VERSION=$(strip $(shell cat ../../fuzzymatch/VERSION))
MIS_VERSION=$(strip $(shell cat ../../mis/VERSION))

# TG Integrated products
RECOMENGINE_TG_VERSION=$(strip $(shell cat ../../plugin/tigergraph/recomengine/VERSION))
COMDETECT_TG_VERSION=$(strip $(shell cat ../../plugin/tigergraph/comdetect/VERSION))
FUZZYMATCH_TG_VERSION=$(strip $(shell cat ../../plugin/tigergraph/fuzzymatch/VERSION))
MIS_TG_VERSION=$(strip $(shell cat ../../plugin/tigergraph/mis/VERSION))

# Pre-built package link
PACKAGE_LINK=$(strip $(shell cat amd-graphanalytics-install-package_link))

VERSION_LIST = 's/%PACKAGE_VERSION/$(PACKAGE_VERSION)/g; \
				s,%PACKAGE_LINK,$(PACKAGE_LINK),g; \
				s/%COSINESIM_VERSION/$(COSINESIM_VERSION)/g; \
				s/%LOUVAIN_VERSION/$(LOUVAIN_VERSION)/g; \
				s/%FUZZYMATCH_VERSION/$(FUZZYMATCH_VERSION)/g; \
				s/%MIS_VERSION/$(MIS_VERSION)/g; \
				s/%RECOMENGINE_TG_VERSION/$(RECOMENGINE_TG_VERSION)/g; \
				s/%COMDETECT_TG_VERSION/$(COMDETECT_TG_VERSION)/g; \
				s/%FUZZYMATCH_TG_VERSION/$(FUZZYMATCH_TG_VERSION)/g; \
				s/%MIS_TG_VERSION/$(MIS_TG_VERSION)/g'
#######################################################################################################################

RST_FILES = index.rst notice.rst tg3-plugins.rst vitis-dev-flow.rst \
    $(COSINESIM_RST_FILES) \
    $(RECOM_TG3_RST_FILES)

.PHONY: doxyrest

doxyrest: .stamp .stamp/cosinesim-doxyrest .stamp/recom-tg3-doxyrest

.stamp/sphinx: doxyrest Makefile-sphinx.mk $(RST_FILES) conf.py
	make -f Makefile-sphinx.mk html && \
	touch .stamp/sphinx

html: cleanhtml .stamp/sphinx update-html-versions

update-html-versions:
	# update product versions in generated html files
	find $(HTML_BUILD_DIR) -type f -exec sed -ie $(VERSION_LIST) {} +

.stamp:
	mkdir -p .stamp


clean: cosinesim-clean recom-tg3-clean cleanhtml
	rm -rf .stamp

cleanhtml:
	rm -rf .stamp/sphinx $(HTML_BUILD_DIR) _build/doctrees

