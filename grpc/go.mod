module grpc-demo

go 1.13

require (
	cloud.google.com/go v0.57.0 // indirect
	github.com/fullstorydev/grpcurl v1.6.0 // indirect
	github.com/golang/protobuf v1.4.0
	google.golang.org/grpc v1.29.1
)

replace google.golang.org/grpc => github.com/grpc/grpc-go v1.29.1
