package main

import (
	"bytes"
	"fmt"
	"log"
	"net"

	"github.com/dictav/test-grpc-flatbuffers/hello"
	"github.com/google/flatbuffers/go"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
)

type helloService struct{}

var message = []byte("hello")

func buildMessage(in *hello.Message) *flatbuffers.Builder {
	b := flatbuffers.NewBuilder(0)
	ns := b.CreateString("GreeterService")
	ts := b.CreateString(fmt.Sprintf("HELLO %s!", in.Name()))
	hello.MessageStart(b)
	hello.MessageAddName(b, ns)
	hello.MessageAddText(b, ts)
	b.Finish(hello.MessageEnd(b))

	return b
}

func (h *helloService) Hello(ctx context.Context, in *hello.Message) (*flatbuffers.Builder, error) {
	if !bytes.Equal(in.Text(), message) {
		return nil, grpc.Errorf(codes.InvalidArgument, "A text should be 'hello'")
	}

	return buildMessage(in), nil
}

func (h *helloService) HelloHello(in *hello.Message, stream hello.Greeter_HelloHelloServer) error {
	if !bytes.Equal(in.Text(), message) {
		return grpc.Errorf(codes.InvalidArgument, "A text should be 'hello'")
	}

	stream.Send(buildMessage(in))
	stream.Send(buildMessage(in))

	return nil
}

func serve() {
	lis, err := net.Listen("tcp", ":9090")
	if err != nil {
		log.Fatal(err)
	}
	s := grpc.NewServer(grpc.CustomCodec(flatbuffers.FlatbuffersCodec{}))
	h := &helloService{}
	hello.RegisterGreeterServer(s, h)
	log.Fatal(s.Serve(lis))
}

func main() {
	serve()
}
