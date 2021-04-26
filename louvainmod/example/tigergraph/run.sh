files=/proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx 
echo $files
time gsql create.gsql
time gsql -g social load.gsql
time gsql -g social "run loading job load_job USING file_name = \"$files\""
time gsql -g social louvain_distributed_cpu.gsql
time gsql -g social louvain_alveo.gsql
START=$(date +%s%3N)
time gsql -g social 'run query louvain_distributed_cpu(10, ["Person"], ["Coworker"], "cpu_out.txt")'
TOTAL_TIME=$(($(date +%s%3N) - START))
echo "louvain_cpu: " $TOTAL_TIME
START=$(date +%s%3N)
#time gsql -g social 'run query load_alveo()'
#TOTAL_TIME=$(($(date +%s%3N) - START))
#echo "load_alveo: " $TOTAL_TIME
#START=$(date +%s%3N)
#time gsql -g social 'run query louvain_alveo(10, ["Person"], ["Coworker"], "alveo_out.txt")'
#TOTAL_TIME=$(($(date +%s%3N) - START))
#echo "louvain_alveo: " $TOTAL_TIME

