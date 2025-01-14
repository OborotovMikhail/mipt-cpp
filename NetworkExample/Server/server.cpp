#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "../Shared/world.h"
#include "../Shared/network.h"
#include "../Shared/viewer.h"
#include "SafeQueue.h"

#include <thread>
#include <iostream>
#include <atomic>

class Server
{
public:
  Server(int port, World& world);
  ~Server();

  bool isRunning() const
  {
    return running;
  }

  std::atomic<bool> dirty = false;

  void update(float dt)
  {
    while (!receivedPackets.empty())
    {
      int messageType;
      int clientId;

      auto packet = receivedPackets.dequeue();
      packet >> messageType;
      
      if (messageType == Message::Movement)
      {
        sf::Vector2f v;
        packet >> clientId >> v.x >> v.y;
        world.players[clientId].v = v;
        dirty = true;
      }
    }

    for (auto& it : world.players)
      it.second.update(dt);
  }

  void synchronize()
  {
    if (!dirty)
      return;
   
    dirty = false;
    std::lock_guard<std::mutex> guard(newPlayerMutex);

    sf::Packet toSend;
    toSend << Message::UpdateWorld << world.players.size();
    for (const auto& [id, player] : world.players)
    {
      toSend << id << player.pos.x << player.pos.y << player.v.x << player.v.y;
    }

    toSend << clock.getElapsedTime().asSeconds();

    for (auto& [id, socket] : sockets)
    {
      if (socket->send(toSend) != sf::Socket::Done)
      {
        std::cout << "Can't send update to player " << id << " \n";
      }
    }
  }

public:
  World& world;

  void receive();

  std::atomic<bool> running = false;

  sf::TcpListener listener;
  sf::SocketSelector selector;

  // Server settings
  const int port;
  const unsigned short MaxPlayers = 3;

  int playersConnected = 0;

  std::unordered_map<PlayerId, std::unique_ptr<sf::TcpSocket>> sockets;

  SafeQueue<sf::Packet> receivedPackets;

  std::thread syncThread;
  std::mutex newPlayerMutex;
  sf::Clock clock;
};

Server::Server(int port, World& world) : 
  port(port), 
  world(world)
{
  if (listener.listen(port) == sf::Socket::Done)
  {
    std::cout << "Server is started on port: " << port << ". Waiting for clients.\n";
    running = true;
  }
  else
  {
    std::cout << "Error - Failed to bind the port " << port << std::endl;
    return;
  }

  syncThread = std::thread(&Server::receive, this);
  syncThread.detach();
}

Server::~Server()
{
  if (running)
  {
    running = false;
    syncThread.join();
  }
}

void Server::receive()
{
  selector.add(listener);

  int currentPlayerId = 0;

  while (isRunning())
  {
    if (selector.wait())
    {
      if (selector.isReady(listener))
      {
        // Creating the socket that will wait for new connections
        auto tempSocket = std::make_unique<sf::TcpSocket>();

        // If a new client is connected this code executes
        if (listener.accept(*tempSocket) == sf::Socket::Done)
        {
          if (playersConnected < MaxPlayers) //if server is not full
          {
            std::lock_guard<std::mutex> guard(newPlayerMutex);

            // The second parameter is the position, change that to some of the spawn positions
            sf::Vector2f pos(float(rand() % World::Size.x), float(rand() % World::Size.y));
            world.players[currentPlayerId] = { pos, sf::Vector2f() };

            selector.add(*tempSocket);
            ++playersConnected;

            sf::Packet outPacket;
            outPacket << Message::ClientCreated << currentPlayerId << pos.x << pos.y << clock.getElapsedTime().asSeconds();

            dirty = true;

            if (tempSocket->send(outPacket) != sf::Socket::Done)
              std::cout << "Error sending player index" << std::endl;
            else
            {
              std::cout << "Player " << currentPlayerId << " connected\n";
              std::cout << "id " << currentPlayerId << " pos: " << pos.x << " " << pos.y << "\n";
            }

            sockets[currentPlayerId] = std::move(tempSocket);
            ++currentPlayerId;
          }
        }
      }
      else
      {
        for (const auto& [id, _] : world.players)
        {
          auto& socket = *sockets[id];
          if (selector.isReady(socket))
          {
            sf::Packet received;
            if (socket.receive(received) == sf::Socket::Done)
            {
              receivedPackets.enqueue(received);
            }
          }
        }
      }
    }
  }
}


int main()
{
  World world;

  Viewer viewer("My server");
  Server server(1234, world);

  sf::Clock gameClock;
  sf::Time tick;

  while (server.isRunning() && viewer.isOpen())
  {
    viewer.handleEvents();

    auto dt = gameClock.restart();
    server.update(dt.asSeconds());

    tick += dt;

    if (tick.asMilliseconds() > 1000 || server.dirty)
    {
      server.synchronize();
      tick = sf::Time();
    }

    viewer.draw(world);
  }

  return 0;
}
