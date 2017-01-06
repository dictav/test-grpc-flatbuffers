all: fbs server client

fbs: hello/Greeter_grpc.go hello_generated.h

hello/Greeter_grpc.go:
	flatc --grpc --go hello.fbs

hello_generated.h:
	flatc --grpc --cpp hello.fbs

server:
	clang++ -std=c++14\
		-L/usr/local/Cellar/grpc/1.0.1/lib\
		-lgrpc++\
		-lgrpc\
		-o server\
		server.cc hello.grpc.fb.cc

client:
	clang++ -std=c++14\
		-L/usr/local/Cellar/grpc/1.0.1/lib\
		-lgrpc++\
		-lgrpc\
		-o client\
		client.cc hello.grpc.fb.cc

clean:
	rm server
	rm client
	rm -rf hello/
	rm hello*.{h,cc}
