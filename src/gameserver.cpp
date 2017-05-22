//
//  gameserver.cpp
//  labyrinth_server
//
//  Created by Aleksandr Borzikh on 28.01.17.
//  Copyright © 2017 sandyre. All rights reserved.
//

#include "gameserver.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include "gsnet_generated.h"

#include "gamelogic/gamemap.hpp"

using namespace GameEvent;
using namespace std::chrono;

GameServer::GameServer(const Configuration& config) :
m_eState(GameServer::State::LOBBY_FORMING),
m_stConfig(config),
m_nStartTime(steady_clock::now()),
m_msPerUpdate(2)
{
    m_sServerName = "GS";
    m_sServerName += std::to_string(m_stConfig.nPort);
    
    m_oLogSys.Init(m_sServerName, LogSystem::Mode::STDIO);
    
    m_oMsgBuilder << "Started. Configuration: {SEED:" << m_stConfig.nRandomSeed;
    m_oMsgBuilder << "; PLAYERS_COUNT: " << m_stConfig.nPlayers << "}";
    m_oLogSys.Write(m_oMsgBuilder.str());
    m_oMsgBuilder.str("");
    
    Poco::Net::SocketAddress addr(Poco::Net::IPAddress(), m_stConfig.nPort);
    m_oSocket.bind(addr);
}

GameServer::~GameServer()
{
    m_oSocket.close();
    m_oLogSys.Close();
}

GameServer::State
GameServer::GetState() const
{
    return m_eState;
}

GameServer::Configuration
GameServer::GetConfig() const
{
    return m_stConfig;
}

void
GameServer::shutdown()
{
    m_eState = GameServer::State::FINISHED;
    
    m_oMsgBuilder << "Finished";
    m_oLogSys.Write(m_oMsgBuilder.str());
    m_oMsgBuilder.str("");
}

void
GameServer::run()
{
    try
    {
            // lobby forming stage
        lobby_forming_stage();
            // hero picking stage
        hero_picking_stage();
            // worldgen stage
        world_generation_stage();
            // running game stage
        running_game_stage();
    }
    catch(std::exception& e)
    {
        m_oMsgBuilder << e.what();
        m_oLogSys.Write(m_oMsgBuilder.str());
        m_oMsgBuilder.str("");
        shutdown();
    }
}

void
GameServer::SendToAll(uint8_t * buf,
                      size_t size)
{
    std::for_each(m_aPlayers.cbegin(),
                  m_aPlayers.cend(),
                  [buf, size, this](const Player& player)
                  {
                      m_oSocket.sendTo(buf, size,
                                       player.GetAddress());
                  });
}

void
GameServer::lobby_forming_stage()
{
    Poco::Net::SocketAddress sender_addr;
    char buf[256];
    
    while(m_eState == State::LOBBY_FORMING)
    {
        auto size = m_oSocket.receiveFrom(buf, 256, sender_addr);
        
        auto gs_event = GetMessage(buf);
        
        if(gs_event->event_type() == Events_CLConnection)
        {
            auto con_info = static_cast<const CLConnection*>(gs_event->event());
            
            auto player = FindPlayerByUID(con_info->player_uid());
            if(player == m_aPlayers.end())
            {
                Player new_player(con_info->player_uid(),
                                  sender_addr,
                                  con_info->nickname()->c_str());
                
                m_oMsgBuilder << "Player \'" << new_player.GetNickname() << "\' connected";
                m_oLogSys.Write(m_oMsgBuilder.str());
                m_oMsgBuilder.str("");
                
                m_aPlayers.emplace_back(std::move(new_player));
                
                    // notify connector that he is accepted
                auto gs_accept = CreateSVConnectionStatus(m_oBuilder,
                                                          ConnectionStatus_ACCEPTED);
                auto gs_event = CreateMessage(m_oBuilder,
                                              Events_SVConnectionStatus,
                                              gs_accept.Union());
                m_oBuilder.Finish(gs_event);
                
                m_oSocket.sendTo(m_oBuilder.GetBufferPointer(),
                                 m_oBuilder.GetSize(),
                                 sender_addr);
                m_oBuilder.Clear();
                
                    // send him info about all current players in lobby
                for(auto& player : m_aPlayers)
                {
                    auto nick = m_oBuilder.CreateString(player.GetNickname());
                    auto player_info = CreateSVPlayerConnected(m_oBuilder,
                                                               player.GetUID(),
                                                               nick);
                    gs_event = CreateMessage(m_oBuilder,
                                             Events_SVPlayerConnected,
                                             player_info.Union());
                    m_oBuilder.Finish(gs_event);
                    
                    m_oSocket.sendTo(m_oBuilder.GetBufferPointer(),
                                     m_oBuilder.GetSize(),
                                     sender_addr);
                    m_oBuilder.Clear();
                }
                
                    // notify all players about connected one
                auto nick = m_oBuilder.CreateString(con_info->nickname()->c_str(),
                                                    con_info->nickname()->size());
                auto gs_new_player = CreateSVPlayerConnected(m_oBuilder,
                                                             con_info->player_uid(),
                                                             nick);
                gs_event = CreateMessage(m_oBuilder,
                                         Events_SVPlayerConnected,
                                         gs_new_player.Union());
                m_oBuilder.Finish(gs_event);
                
                SendToAll(m_oBuilder.GetBufferPointer(),
                          m_oBuilder.GetSize());
                m_oBuilder.Clear();
            }
        }
        
        if(m_aPlayers.size() == m_stConfig.nPlayers)
        {
            m_eState = GameServer::State::HERO_PICK;
            
            auto gs_heropick = CreateSVHeroPickStage(m_oBuilder);
            auto gs_event = CreateMessage(m_oBuilder,
                                          Events_SVHeroPickStage,
                                          gs_heropick.Union());
            m_oBuilder.Finish(gs_event);
            
            SendToAll(m_oBuilder.GetBufferPointer(),
                      m_oBuilder.GetSize());
            m_oBuilder.Clear();
        }
    }
}

void
GameServer::hero_picking_stage()
{
    Poco::Net::SocketAddress sender_addr;
    char buf[256];
    
    while(m_eState == State::HERO_PICK)
    {
        auto size = m_oSocket.receiveFrom(buf, 256, sender_addr);
        
        auto gs_event = GetMessage(buf);
        
        switch(gs_event->event_type())
        {
            case GameEvent::Events_CLHeroPick:
            {
                auto cl_pick = static_cast<const CLHeroPick*>(gs_event->event());
                
                auto player = FindPlayerByUID(cl_pick->player_uid());
                player->SetHeroPicked((Hero::Type)cl_pick->hero_type());
                
                auto sv_pick = CreateSVHeroPick(m_oBuilder,
                                                cl_pick->player_uid(),
                                                cl_pick->hero_type());
                auto sv_event = CreateMessage(m_oBuilder,
                                              Events_SVHeroPick,
                                              sv_pick.Union());
                m_oBuilder.Finish(sv_event);
                SendToAll(m_oBuilder.GetBufferPointer(),
                          m_oBuilder.GetSize());
                m_oBuilder.Clear();
                
                break;
            }
                
            case GameEvent::Events_CLReadyToStart:
            {
                auto cl_ready = static_cast<const CLReadyToStart*>(gs_event->event());
                
                auto player = FindPlayerByUID(cl_ready->player_uid());
                player->SetState(Player::State::PRE_READY_TO_START);
                
                auto sv_ready = CreateSVReadyToStart(m_oBuilder,
                                                     cl_ready->player_uid());
                auto sv_event = CreateMessage(m_oBuilder,
                                              Events_SVReadyToStart,
                                              sv_ready.Union());
                m_oBuilder.Finish(sv_event);
                SendToAll(m_oBuilder.GetBufferPointer(),
                          m_oBuilder.GetSize());
                m_oBuilder.Clear();
                
                break;
            }
                
            default:
                assert(false);
                break;
        }
        
        bool bEveryoneReady = std::all_of(m_aPlayers.begin(),
                                          m_aPlayers.end(),
                                          [](Player& player)
                                          {
                                              return player.GetState() == Player::State::PRE_READY_TO_START;
                                          });
        if(bEveryoneReady)
        {
            m_eState = GameServer::State::GENERATING_WORLD;
            
            m_oMsgBuilder << "Generating world";
            m_oLogSys.Write(m_oMsgBuilder.str());
            m_oMsgBuilder.str("");
            
            for(auto& player : m_aPlayers)
            {
                player.SetState(Player::State::IN_GAME);
            }
        }
    }
}

void
GameServer::world_generation_stage()
{
    Poco::Net::SocketAddress sender_addr;
    char buf[256];
    
    while(m_eState == State::GENERATING_WORLD)
    {
        GameMap::Configuration sets;
        sets.nSeed = m_stConfig.nRandomSeed;
        sets.nSeed = m_stConfig.nRandomSeed;
        sets.nMapSize = 3;
        sets.nRoomSize = 10;
        m_pGameWorld = std::make_unique<GameWorld>();
        m_pGameWorld->SetLoggingSystem(&m_oLogSys);
        m_pGameWorld->CreateGameMap(sets);
        
        for(int i = 0; i < m_aPlayers.size(); ++i)
        {
            m_pGameWorld->AddPlayer(m_aPlayers[i]);
        }
        
        m_pGameWorld->InitialSpawn();
        
        auto gs_gen_map = CreateSVGenerateMap(m_oBuilder,
                                              sets.nMapSize,
                                              sets.nRoomSize,
                                              sets.nSeed);
        auto gs_event = CreateMessage(m_oBuilder,
                                      Events_SVGenerateMap,
                                      gs_gen_map.Union());
        m_oBuilder.Finish(gs_event);
        
        SendToAll(m_oBuilder.GetBufferPointer(), m_oBuilder.GetSize());
        m_oBuilder.Clear();
        
        auto players_ungenerated = m_aPlayers.size();
        while(players_ungenerated)
        {
            m_oSocket.receiveBytes(buf, 256);
            
            auto gs_event = GetMessage(buf);
            
            if(gs_event->event_type() == Events_CLMapGenerated)
            {
                auto cl_gen_ok = static_cast<const CLMapGenerated*>(gs_event->event());
                
                m_oMsgBuilder << "Player " << cl_gen_ok->player_uid();
                m_oMsgBuilder << " generated map";
                m_oLogSys.Write(m_oMsgBuilder.str());
                m_oMsgBuilder.str("");
                
                --players_ungenerated;
            }
        }
        
        m_eState = GameServer::State::RUNNING_GAME;
        
            // notify players that game starts!
        auto game_start = CreateSVGameStart(m_oBuilder);
        gs_event = CreateMessage(m_oBuilder,
                                 Events_SVGameStart,
                                 game_start.Union());
        m_oBuilder.Finish(gs_event);
        SendToAll(m_oBuilder.GetBufferPointer(), m_oBuilder.GetSize());
        m_oBuilder.Clear();
        
        m_oMsgBuilder << "Game begins";
        m_oLogSys.Write(m_oMsgBuilder.str());
        m_oMsgBuilder.str("");
    }
}

void
GameServer::running_game_stage()
{
    char buf[256];
    Poco::Net::SocketAddress sender_addr;
    std::chrono::milliseconds time_no_receive = 0ms;
    
    while(m_eState == State::RUNNING_GAME)
    {
        auto frame_start = steady_clock::now();
        
            // sleep for some time, then get all packets and pass it to the gameworld, update
        std::this_thread::sleep_for(m_msPerUpdate);
        
        if(!m_oSocket.available())
        {
            time_no_receive += m_msPerUpdate;
        }
        while(m_oSocket.available())
        {
            time_no_receive = 0ms;
            bool event_valid = false;
            auto pack_size = m_oSocket.receiveFrom(buf,
                                                   256,
                                                   sender_addr);
            
            auto gs_event = GetMessage(buf);
            
            switch(gs_event->event_type())
            {
                case Events_CLActionMove:
                {
                    auto cl_mov = static_cast<const CLActionMove*>(gs_event->event());
                    auto player = FindPlayerByUID(cl_mov->target_uid());
                    
                    if(player != m_aPlayers.end() &&
                       player->GetAddress() != sender_addr)
                    {
                        player->SetAddress(sender_addr);
                    }
                    
                    event_valid = true;
                    break;
                }
                    
                case Events_CLActionItem:
                {
                    auto cl_item = static_cast<const CLActionItem*>(gs_event->event());
                    auto player = FindPlayerByUID(cl_item->player_uid());
                    
                    if(player != m_aPlayers.end() &&
                       player->GetAddress() != sender_addr)
                    {
                        player->SetAddress(sender_addr);
                    }
                    
                    event_valid = true;
                    break;
                }
                    
                case Events_CLActionDuel:
                {
                    auto cl_duel = static_cast<const CLActionDuel*>(gs_event->event());
                    
                        //FIXME: players validation needed?
                    
                    event_valid = true;
                    break;
                }
                    
                case Events_CLActionSpell:
                {
                        //FIXME: validation needed, at least for players id
                    event_valid = true;
                    break;
                }
                    
                case Events_CLActionAttack:
                {
                    event_valid = true;
                    break;
                }
                    
                default:
                    assert(false);
                    break;
            }
            
            if(event_valid) // add received event to the gameworld
            {
                auto& in_events = m_pGameWorld->GetIncomingEvents();
                std::vector<uint8_t> event(buf,
                                           buf + pack_size);
                in_events.emplace(event);
            }
        }
        
        auto frame_end = steady_clock::now();
        
        m_pGameWorld->update(duration_cast<milliseconds>(frame_end-frame_start));
        
        auto& out_events = m_pGameWorld->GetOutgoingEvents();
        while(out_events.size())
        {
            auto event = out_events.front();
            SendToAll(event.data(), event.size());
            out_events.pop();
        }
        
        if(time_no_receive >= 30s)
        {
            m_oMsgBuilder << "Server timeout exceeded.";
            m_oLogSys.Write(m_oMsgBuilder.str());
            m_oMsgBuilder.str("");
            shutdown();
        }
    }
}

void
GameServer::SendToOne(uint32_t player_id,
                      uint8_t * buf,
                      size_t size)
{
    auto player = FindPlayerByUID(player_id);
    
    if(player != m_aPlayers.end())
    {
        m_oSocket.sendTo(buf,
                         size,
                         player->GetAddress());
    }
}

std::vector<Player>::iterator
GameServer::FindPlayerByUID(PlayerUID uid)
{
    for(auto iter = m_aPlayers.begin();
        iter != m_aPlayers.end();
        ++iter)
    {
        if((*iter).GetUID() == uid)
        {
            return iter;
        }
    }
    
    return m_aPlayers.end();
}
