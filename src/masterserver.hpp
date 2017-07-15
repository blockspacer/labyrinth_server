//
//  masterserver.hpp
//  labyrinth_server
//
//  Created by Aleksandr Borzikh on 28.01.17.
//  Copyright © 2017 sandyre. All rights reserved.
//

#ifndef masterserver_hpp
#define masterserver_hpp

#include "GameServersController.hpp"
#include "msnet_generated.h"
#include "services/system_monitor.hpp"
#include "services/DatabaseAccessor.hpp"
#include "toolkit/named_logger.hpp"

#include <Poco/Data/SessionFactory.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/TaskManager.h>
#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>

#include <array>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

class MasterServer : public Poco::Runnable
{
private:
    class RegistrationTask;
    class LoginTask;
    class FindGameTask;

    using GameServers = std::vector<std::unique_ptr<GameServer>>;

public:
    struct SystemStatus
    {
        static const int NETWORK_SYSTEM_ACTIVE  = 0x02;
        static const int DATABASE_SYSTEM_ACTIVE = 0x08;
    };

public:
    MasterServer();
    ~MasterServer();

    void init(uint32_t Port);

    virtual void run();

protected:
    void service_loop();

protected:
    std::mt19937                    _randGenerator;
    std::uniform_int_distribution<> _randDistr;

    std::queue<uint32_t>            _availablePorts;

    uint32_t                        _systemStatus;

    // Gameservers
    std::unique_ptr<GameServersController>  _gameserversController;

    // Network
    Poco::Net::DatagramSocket      _socket;
    flatbuffers::FlatBufferBuilder _flatBuilder;
    std::array<uint8_t, 512>       _dataBuffer;

    // Logging
    NamedLogger                          _logger;

    Poco::ThreadPool                     _taskWorkers;
    Poco::TaskManager                    _taskManager;

    // Services
    std::unique_ptr<SystemMonitor>       _systemMonitor;

    friend RegistrationTask;
    friend LoginTask;
    friend FindGameTask;
};

#endif /* masterserver_hpp */
