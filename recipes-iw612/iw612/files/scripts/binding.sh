#!/bin/bash -x
# =============================================
# Parameters
#

if [ $# -lt 2 ]; then
	echo "Usage:"
	echo "./binding.sh switch_id light_id"
	echo "switch_id: switch node id"
	echo "light_id: light node id"
	exit
fi

switch_nodeid=$1
light_nodeid=$2

chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": ['$switch_nodeid'], "targets": null }]' $light_nodeid 0
chip-tool binding write binding '[{"fabricIndex": 1, "node":'$light_nodeid', "endpoint": 1, "cluster": 6}]' $switch_nodeid 1 
