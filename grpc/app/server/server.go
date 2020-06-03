package main

import (
    "context"
    "fmt"
    "google.golang.org/grpc"
    "grpc-demo/gym"
    "io"
    "log"
    "net"
)

const (
    ADDR = ":50051"
)

// server继承自动生成的服务类
type server struct {
    gym.UnimplementedGymServer
}

func (s *server) Building(ctx context.Context, in *gym.Builder) (*gym.Reply, error) {
    fmt.Printf("%s正在健身, 动作: %s\n", in.Name, in.Actions)
    return &gym.Reply{Code: 0, Msg: "ok"}, nil
}

func (s *server) Talking(ctx context.Context, in *gym.Talker) (*gym.Reply, error) {
    fmt.Printf("%s正在划水: %s\n", in.Name, in.Content)
    return &gym.Reply{Code: 0, Msg: "ok"}, nil
}

func (s *server) Pay(srv gym.Gym_PayServer) error {
    for {
        in, err := srv.Recv()
        if err == io.EOF {
            return nil
        }
        if err != nil {
            log.Printf("failed to recv: %v", err)
            return err
        }
        fmt.Printf("%s正在充钱: %d\n", in.Name, in.Amount)
        srv.Send(&gym.Reply{Code: 0, Msg: "ok"})
    }
}

func main() {
    lis, err := net.Listen("tcp", ADDR)
    if err != nil {
        log.Fatalf("failed to listen: %v", err)
    }

    s := grpc.NewServer()
    gym.RegisterGymServer(s, &server{})

    if err := s.Serve(lis); err != nil {
        log.Fatalf("failed to serve: %v", err)
    }
}