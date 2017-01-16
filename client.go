package main

import (
	"io"
	"log"

	"github.com/dictav/test-grpc-flatbuffers/hello"
	"github.com/google/flatbuffers/go"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
)

func buildMessage(name, text string) *flatbuffers.Builder {
	b := flatbuffers.NewBuilder(0)
	ns := b.CreateString(name)
	ts := b.CreateString(text)
	hello.MessageStart(b)
	hello.MessageAddName(b, ns)
	hello.MessageAddText(b, ts)
	b.Finish(hello.MessageEnd(b))

	return b
}

func printReply(reply *hello.Message, err error) {
	if err != nil {
		c := grpc.Code(err)
		d := grpc.ErrorDesc(err)
		if c != codes.Unknown {
			log.Printf("=> Error: code=%d(%s), desc=%s\n", c, c, d)
			return
		}

		if err != io.EOF {
			log.Println("=> Error: not rpc error=", err)
		}

		return
	}

	if reply == nil {
		log.Println("=> Unexpected Error: message is nil")
		return
	}

	log.Printf("=> %s From %s\n", reply.Text(), reply.Name())
}

func main() {
	conn, err := grpc.Dial(":9090", grpc.WithInsecure(), grpc.WithCodec(flatbuffers.FlatbuffersCodec{}))
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()
	client := hello.NewGreeterClient(conn)

	ctx := context.Background()

	log.Println("Say hello From dictav")
	rep, err := client.Hello(ctx, buildMessage("dictav", "hello"))
	printReply(rep, err)

	log.Println("Say goodbye From dictav")
	rep, err = client.Hello(ctx, buildMessage("dictav", "goodbye"))
	printReply(rep, err)

	log.Println("Say hello From dictav (HelloHello)")
	strm, err := client.HelloHello(ctx, buildMessage("dictav", "hello"))
	if err != nil {
		printReply(nil, err)
	}
	for {
		rep, err = strm.Recv()
		printReply(rep, err)
		if err != nil {
			break
		}
	}
}
