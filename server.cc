#include <grpc++/grpc++.h>

#include "hello_generated.h"
#include "hello.grpc.fb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using hello::Message;
using hello::Greeter;

class GreeterServiceImpl final : public hello::Greeter::Service {
  Status Hello(ServerContext* context,
      const flatbuffers::BufferRef<Message>* request,
      flatbuffers::BufferRef<Message>* reply)
      override {
    if (request->GetRoot()->text()->str() != "hello") {
      return Status(grpc::INVALID_ARGUMENT, "A text should be 'hello'");
    }

    // Build a reply with the name and the text.
    flatbuffers::FlatBufferBuilder fbb;
    auto msg_offset = hello::CreateMessage(fbb,
        fbb.CreateString("GreeterService"),
        fbb.CreateString("HELLO " + request->GetRoot()->name()->str() + "!")
        );
    fbb.Finish(msg_offset);

    *reply = flatbuffers::BufferRef<Message>(fbb.GetBufferPointer(), fbb.GetSize());

    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:9090");
  GreeterServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
