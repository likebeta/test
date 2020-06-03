#!/usr/bin/env bash

protoDir="../protos"
outDir="../gym"
protoc -I ${protoDir}/ ${protoDir}/*proto --gofast_out=plugins=grpc:${outDir}
