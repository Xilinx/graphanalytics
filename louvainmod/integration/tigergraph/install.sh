Time=$(date +%s%3N)
echo "/data/dxgradb/tigergraph/tigergraph/app/3.1.0/dev/gdk/MakeUdf.$Time"
# Create backups
grun all "cp /data/dxgradb/tigergraph/tigergraph/app/3.1.0/dev/gdk/MakeUdf /data/dxgradb/tigergraph/tigergraph/app/3.1.0/dev/gdk/MakeUdf.$Time"

# Override with new ones
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/makefiles/MakeUdf /data/dxgradb/tigergraph/tigergraph/app/3.1.0/dev/gdk"

grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/ExprFunctions.hpp /data/dxgradb/tigergraph/tigergraph/data/gsql/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/ExprUtil.hpp /data/dxgradb/tigergraph/tigergraph/data/gsql/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/xaidecl.cpp /data/dxgradb/tigergraph/tigergraph/data/gsql/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/xai.h /data/dxgradb/tigergraph/tigergraph/data/gsql/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/xailoader.hpp /data/dxgradb/tigergraph/tigergraph/data/gsql/udf"

grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/ExprFunctions.hpp /home2/tigergraph/tigergraph/tmp/gsql/codegen/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/ExprUtil.hpp /home2/tigergraph/tigergraph/tmp/gsql/codegen/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/xaidecl.cpp /home2/tigergraph/tigergraph/tmp/gsql/codegen/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/xai.h /home2/tigergraph/tigergraph/tmp/gsql/codegen/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/integration/tigergraph/udf/QueryUdf/xailoader.hpp /home2/tigergraph/tigergraph/tmp/gsql/codegen/udf"
grun all "cp /proj/isimsj/graphdb/louvain/poc_louvain/L3/lib/libgraphL3.so /data/dxgradb/tigergraph/tigergraph/data/gsql/udf"

