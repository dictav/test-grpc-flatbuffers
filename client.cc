#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include <grpc++/grpc++.h>

#include "hello_generated.h"
#include "hello.grpc.fb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using hello::Message;
using hello::Greeter;

class GreeterClient {
  public:
    GreeterClient(shared_ptr<Channel> channel) : stub_(Greeter::NewStub(channel)) {}

    string Hello(const string& name, const string& text) {

      auto request = buildMessage(name, text);
      flatbuffers::BufferRef<Message> reply;

      ClientContext context;
      Status status = stub_->Hello(&context, request, &reply);

      ostringstream strm;
      if (status.ok()) {
        strm << "=> " << reply.GetRoot()->text()->str() << " From " << reply.GetRoot()->name()->str();
      } else {
        strm << "=> Error: code=" << status.error_code() << ", desc=" << status.error_message();
      }
      return strm.str();
    }

    string HelloHello(const string& name, const string& text) {
      auto request = buildMessage(name, text);

      ClientContext context;
      auto reader = stub_->HelloHello(&context, request); 

      ostringstream strm;
      flatbuffers::BufferRef<Message> reply;
      while (reader->Read(&reply)) {
        strm << "=> " << reply.GetRoot()->text()->str() << " From " << reply.GetRoot()->name()->str() << endl;
      }
      Status status = reader->Finish();
      if (!status.ok()) {
        strm << "=> Error: code=" << status.error_code() << ", desc=" << status.error_message();
      }

      return strm.str();
    }

  private:
    unique_ptr<hello::Greeter::Stub> stub_;

    flatbuffers::BufferRef<Message> buildMessage(string name, const string text) {
      flatbuffers::FlatBufferBuilder fbb;
      auto msg_offset = hello::CreateMessage(fbb,
          fbb.CreateString(name), 
          fbb.CreateString(text));
      fbb.Finish(msg_offset);
      return flatbuffers::BufferRef<Message>(fbb.GetBufferPointer(), fbb.GetSize());
    }
};

int main(int argc, char** argv) {
  GreeterClient hello(grpc::CreateChannel("localhost:9090", grpc::InsecureChannelCredentials()));

  cout << "Say hello From dictav" << endl;
  string reply = hello.Hello("dictav", "hello");
  cout <<  reply << endl;

  cout << "Say hello From dictav" << endl;
  reply = hello.Hello("dictav", "goodbye");
  cout <<  reply << endl;

  cout << "Say hello From dictav (HelloHello)" << endl;
  reply = hello.HelloHello("dictav", "hello");
  cout <<  reply << endl;

  return 0;
}
