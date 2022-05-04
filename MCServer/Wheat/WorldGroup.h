#pragma once
#include "Config.h"
#include "WebAPI.h"
class World {
public:
	string name;
	string IP;
	int port;
};

class WorldManager{
public:
	std::list<World*> worldList;

	WorldManager() {
		World* w = new World;
		w->name = "SC";
		w->IP = "114.132.244.225";
		w->port = 20000;
		worldList.push_back(w);
	};
	void teleportPlayer(Player* pl, string IP, int port) {
		pl->transferServer(IP, port);
	}
	void teleportPlayerByWorldName(Player* pl, string name) {
		World* w = getWorldByName(name);
		teleportPlayer(pl, w->IP, w->port);
	}
	World* getWorldByName(string name) {
		for (World* w : worldList) {
			if (w->name == name) {
				return w;
			}
		}
		return nullptr;
	}
} worldManager;

