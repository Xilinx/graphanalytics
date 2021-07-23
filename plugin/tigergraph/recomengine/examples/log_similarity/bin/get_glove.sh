#!/bin/bash
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
#
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
#

if [ ! -f "/tmp/glove.6B.50d.txt" ]; then
    if [ ! -f "/tmp/glove.6B.50d.txt.tar" ]; then
        echo "Downloading GloVe Embedding File ..."
        wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=1ogyMmAu0fcZBdSwTQJuX6jHLzlTJnql0' -O /tmp/glove.6B.50d.txt.tar
        echo "Download Completed!"
    fi
    echo "Extracting GloVe Embedding File ..."
    tar -xvzf /tmp/glove.6B.50d.txt.tar -C /tmp/
    chmod 777 /tmp/glove.6B.50d.txt
    echo "Done!"
fi