#!/bin/bash

AGENT_NAME_PATTERN="nvme_agent"

# unceremoniously kill the agent
pkill -9 ${AGENT_NAME_PATTERN}