#!/usr/bin/env bash 

SCRIPT=$(readlink -f $0)
script_dir=`dirname $SCRIPT`

function usage() {
    echo "Usage: $0 -u TG-username -p TG-password [optional options]"
    echo "Optionally optional options:"
    echo "  -d : Skip doing drop all"
    echo "  -h : Display this help message"
}

doDropAll=1

while getopts "dh" opt
do
case $opt in
    d) doDropAll=0;;
    h) usage; exit 0;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done


if ! [ -x "$(command -v jq)" ]; then
    echo "ERROR: The program jq is required. Please follow the instructions below to install it:"
    echo "       RedHat/CentOS: sudo yum install jq"
    echo "       Ubuntu: sudo apt-get install jq"
    exit 1
fi

if [ ! -f "$HOME/.tg.cfg" ]; then
    echo "ERROR: Please ensure that you are running this script as tigergraph"
    echo "       and that you are running TigerGraph version 3.x"
    echo "INFO: Installed version:"
    gadmin version | grep TigerGraph
    exit 1
fi
tg_app_root=$(cat $HOME/.tg.cfg | jq .System.AppRoot | tr -d \")
tg_temp_root=$(cat $HOME/.tg.cfg | jq .System.TempRoot | tr -d \")
tg_data_root=$(cat $HOME/.tg.cfg | jq .System.DataRoot | tr -d \")

# set up PATH for tigergraph commands
export PATH=$tg_root_dir/../cmd:$PATH

#
# Clean ExprFunctions.hpp
#

echo "INFO: Cleaning ExprFunctions.hpp..."
expr_functions_dir=$tg_data_root/gsql/udf

# If ExprFunctions.hpp.orig exists, replace ExprFunctions.hpp with it

if [ -f $expr_functions_dir/ExprFunctions.hpp.orig ]; then
    cp -f $expr_functions_dir/ExprFunctions.hpp.orig $expr_functions_dir/ExprFunctions.hpp
    echo "INFO: Replaced ExprFunctions.hpp with ExprFunctions.hpp.orig"

# No ExprFunctions.hpp.orig: if there are no mergeHeaders tags, we can leave ExprFunctions.hpp alone;
# Otherwise, the user will have to clean up ExprFunctions.hpp manually

else
    if [ -f $expr_functions_dir/ExprFunctions.hpp ]; then
        if [ $(grep -c "mergeHeaders" $expr_functions_dir/ExprFunctions.hpp) -gt 0 ]; then
            echo "ERROR: ExprFunctions.hpp has been modified and there is no ExprFunctions.hpp.orig."
            echo "       The file will need to be restored manually."
            exit 1
        else
            echo "INFO: ExprFunctions.hpp appears not to have any plug-ins installed.  Leaving the file as is."
        fi
    else
        echo "INFO: There is no ExprFunctions.hpp to clean."
    fi
fi

#
# Clean the TG database
#

if [ $doDropAll -eq 1 ]; then
    echo "INFO: Dropping all items from TigerGraph database..."
    gsql drop all
else
    echo "INFO: Skipping drop all"
fi

#
# Clean out UDF .so's
#

echo "INFO: Cleaning out UDF .so libraries..."
rm -rf $tg_app_root/bin/libudf/*.so* $tg_app_root/dev/gdk/objs/gq_*.o

echo "INFO: Done"
