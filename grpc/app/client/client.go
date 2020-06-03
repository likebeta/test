package main

import (
    "context"
    "flag"
    "fmt"
    "google.golang.org/grpc"
    "grpc-demo/gym"
    "log"
    "time"
)

const (
    ADDR = "localhost:50051"
)

func main() {
    t := flag.String("t", "talk", "类型")
    flag.Parse()

    // Set up a connection to the server.
    conn, err := grpc.Dial(ADDR, grpc.WithInsecure(), grpc.WithBlock())
    if err != nil {
        log.Fatalf("did not connect: %v", err)
    }
    defer conn.Close()
    c := gym.NewGymClient(conn)

    ctx, cancel := context.WithTimeout(context.Background(), time.Second)
    defer cancel()

    var r *gym.Reply
    if *t == "build" {
        if r, err = c.Building(ctx, &gym.Builder{
            Name:    "yi",
            Actions: []string{"深蹲", "卧推", "硬拉"},
        }); err == nil {
            fmt.Println("build recv: ", r.Code, r.Msg)
        }
    } else if *t == "talk" {
        r, err = c.Talking(ctx, &gym.Talker{
            Name:    "wang",
            Content: "划水划水",
        })
        fmt.Println("talk recv: ", r.Code, r.Msg)
    } else {
        var stream gym.Gym_PayClient
        if stream, err = c.Pay(ctx); err == nil {
            for i := uint32(0); i < 3; i++ {
                if err = stream.Send(&gym.Payer{Name: "li", Amount: i*10 + 5}); err != nil {
                    break
                }
                var r *gym.Reply
                if r, err = stream.Recv(); err != nil {
                    break
                }
                fmt.Println("pay recv: ", r.Code, r.Msg)
            }
            if err == nil {
                stream.CloseSend()
            }
        }
    }
    if err != nil {
        log.Fatalf("error: %v", err)
    }
}
