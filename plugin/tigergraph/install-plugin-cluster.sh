#/bin/bash

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

function usage() {
    echo "Usage: $0 [optional options]"
    echo "Optional options:"
    echo "  -i sshKey           : SSH key for user tigergraph"    
    echo "  -h                  : Print this help message"
}

hostname=$(hostname)
# set default ssh_key for tigergraph
if [ -f ~/.ssh/tigergraph_rsa ]; then
    ssh_key_flag="-i ~/.ssh/tigergraph_rsa"
fi

uninstall_flag=
while getopts ":i:uh" opt
do
case $opt in
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    u) uninstall_flag=-u;;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

ssh $ssh_key_flag tigergraph@$hostname grun all \"${SCRIPTPATH}/install-plugin-node.sh $uninstall_flag\"
