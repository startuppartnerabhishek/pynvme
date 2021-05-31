#!/bin/bash

AGENT_NAME_PATTERN="nvme_agent"

echo "pkill-ing agent pattern ${AGENT_NAME_PATTERN}"

# unceremoniously kill the agent
pkill -9 ${AGENT_NAME_PATTERN}

echo "Older agent instance(s) killed"
