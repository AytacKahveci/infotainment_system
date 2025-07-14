#include <memory>
#include "media_service.h"

using namespace media_service;

int main()
{
  std::vector<Song> songs{
    Song(1, "Track1", "Artist1", 120000),
    Song(2, "Track2", "Artist2", 140000),
    Song(3, "Track3", "Artist3", 110000),
  };
  
  std::string serverAddress{"0.0.0.0:50051"};
  auto mediaService = std::make_unique<MediaServiceImpl>(songs);

  ServerBuilder builder;
  builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
  builder.RegisterService(mediaService.get());

  auto server = builder.BuildAndStart();
  std::cout << "Server listening on: " << serverAddress << std::endl;

  server->Wait();

  return 0;
}
