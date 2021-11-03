# Copyright (C) 2021 Xilinx, Inc
#
# Licensed under the Apache License, Version 2.0 (the "License"). You may
# not use this file except in compliance with the License. A copy of the
# License is located at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
#
PKGDIR := PKG_Release
RM := rm -rf
PROD_VERSION := $(strip $(shell cat ../staging/VERSION))

OSDIST = $(shell lsb_release -si)
ifeq ($(OSDIST),Ubuntu)
    DIST_TARGET = deb
else ifeq ($(OSDIST),CentOS)
    DIST_TARGET = rpm
endif

package:$(PKGDIR)
	cd $(PKGDIR); \
	cmake -DCMAKE_BUILD_TYPE=Release -DPROD_VERSION=${PROD_VERSION} -DCMAKE_INSTALL_PREFIX=/opt/xilinx/apps/graphanalytics/  ..;\
	cmake --build .;\
	cpack -B $(shell pwd) ; \
	cd ..; \
	rm ${PKGDIR} -r; 


$(PKGDIR):
	mkdir $(PKGDIR); 

clean:
	rm -rf _CPack_Packages
