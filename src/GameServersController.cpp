//
//  GameServersController.cpp
//  labyrinth_serv_xcode
//
//  Created by Aleksandr Borzikh on 10.07.17.
//  Copyright © 2017 hate-red. All rights reserved.
//

#include "GameServersController.hpp"

#include <Poco/Observer.h>

GameServersController::GameServersController()
: _logger("GameServersController", NamedLogger::Mode::STDIO),
  _workers("GameServerWorker"),
  _taskManager(_workers)
{
    _logger.Debug() << "GameServerController is up, number of workers: " << _workers.available();

    using namespace Poco;
    _taskManager.addObserver(Observer<GameServersController, TaskStartedNotification>(*this,
                                                                                      &GameServersController::onStarted));
    _taskManager.addObserver(Observer<GameServersController, TaskFinishedNotification>(*this,
                                                                                       &GameServersController::onFinished));
}

std::experimental::optional<uint16_t>
GameServersController::GetServerAddress()
{
    std::experimental::optional<uint16_t> result;
    auto list = _taskManager.taskList();
    for(auto task : list)
    {
        if(task->state() == Poco::Task::TaskState::TASK_IDLE)
            return dynamic_cast<GameServer*>(task.get())->GetConfig().Port;
    }

        // If no servers available - start new one and return its address
    if(!result && _workers.available())
    {
        GameServer::Configuration config;
        config.Players    = 1;
        config.RandomSeed = 0;
        config.Port       = 1931;
        _taskManager.start(new GameServer(config));

        result = config.Port;
    }

    return result;
}
