#/bin/bash

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

while getopts ":i:h" opt
do
case $opt in
    i) ssh_key=$OPTARG; ssh_key_flag="-i $ssh_key";;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

ssh $ssh_key_flag tigergraph@$hostname grun all /proj/gdba/ywu/ghe/graphanalytics/plugin/tigergraph/install-plugin-node.sh