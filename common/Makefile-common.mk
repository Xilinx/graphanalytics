#
# Copyright 2020-2021 Xilinx, Inc.
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

###############################################################################
# Common targets for all products
###############################################################################
#
# Packaging
#

OSDIST = $(shell lsb_release -si)
OSVER = $(shell lsb_release -sr)
OSVER_MAJOR = $(shell lsb_release -sr | tr -dc '0-9.' | cut -d \. -f1)
OSVER_MINOR = $(shell lsb_release -sr | tr -dc '0-9.' | cut -d \. -f2)
OSVER_DIR=$(OSVER_MAJOR).$(OSVER_MINOR)
DIST_TARGET =
OSDISTLC =
ifeq ($(OSDIST),Ubuntu)
    DIST_TARGET = deb
	OSDISTLC = ubuntu
else ifeq ($(OSDIST),CentOS)
    DIST_TARGET = rpm
	OSDISTLC = centos
endif

ARCH = $(shell uname -p)
CPACK_PACKAGE_FILE_NAME= xilinx-$(STANDALONE_NAME)-$(PRODUCT_VER)_$(OSVER)-$(ARCH).$(DIST_TARGET)
DIST_INSTALL_DIR = $(GRAPH_ANALYTICS_DIR)/scripts/xilinx-tigergraph-install/$(OSDISTLC)-$(OSVER_DIR)/$(STANDALONE_NAME)/

.PHONY: dist

dist: stage
	@if [ "$(DIST_TARGET)" == "" ]; then \
	    echo "Packaging is supported for only Ubuntu and CentOS."; \
	else \
	    echo "Packaging $(DIST_TARGET) for $(OSDIST)"; \
	    cd package; \
		make ; \
		cd - ; \
		cp ./package/$(CPACK_PACKAGE_FILE_NAME) $(DIST_INSTALL_DIR); \
		echo "INFO: $(CPACK_PACKAGE_FILE_NAME) saved to $(DIST_INSTALL_DIR)"; \
	fi