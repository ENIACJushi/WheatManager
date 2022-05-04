#pragma once
#include "Config.h"

class playerCache_ {
public:
	map<string, int>    money;           // xuid - money
	std::list<std::pair<string, string>>   refusedPlayer;      // (xuid,message), (xuid,message)...
	std::list<string>   synchronizedPlayer; // xuid, xuid...
	std::list<std::pair<string,int>>   transformingPlayer; // name-timestamp, name-timestamp... 因为拦截时按name拦截
	map<string, string> tags;            // xuid - tags
	map<string, string> sNBT; // xuid -NBT

	void pushRefusedPlayer(string xuid, string serverName) {
		refusedPlayer.push_back(std::pair(xuid, serverName));
	}
	std::pair<bool, string> findRefusedPlayer(string xuid, bool del = false) {
		for (std::list<std::pair<string, string>>::iterator id = playerCache.refusedPlayer.begin();
			id != refusedPlayer.end(); id++) {
			if (xuid == id->first) {
				if (del) refusedPlayer.erase(id);
				return std::pair(true, id->second);
			}
		}
		return std::pair(false, "null");
	}

	void pushSynchronizedPlayer(string xuid) {
		synchronizedPlayer.push_back(xuid);
	}
	bool findSynchronizedPlayer(string xuid, bool del = false) {
		for (std::list<string>::iterator id = playerCache.synchronizedPlayer.begin();
			id != synchronizedPlayer.end(); id++) {
			if (xuid == id->c_str()) {
				if (del) synchronizedPlayer.erase(id);
				return true;
			}
		}
		return false;
	}

	void pushTransformingPlayer(string xuid) {
		transformingPlayer.push_back(std::pair(xuid, 2));
	}
	bool findTransformingPlayer(string n, bool del = false) {
		for (std::list<std::pair<string, int>>::iterator name = transformingPlayer.begin();
			name != transformingPlayer.end(); name++) {
			if (n == name->first) {
				if (del) transformingPlayer.erase(name);
				return true;
			}
		}
		return false;
	}

	void pushMoney(string xuid, int m) {
		money[xuid] = m;
	}
	int getMoney(string xuid, bool del = true) {
		std::map<string, int>::iterator it = money.find(xuid);
		if (it != money.end()) {
			if (del) {
				int result = it->second;
				money.erase(it);
				return result;
			}
			else {
				return it->second;
			}
		}
		else {
			return -9961;
		}
	}

	void pushTags(string xuid, string tag) {
		tags[xuid] = tag;
	}
	string getTags(string xuid, bool del = true) {
		std::map<string, string>::iterator it = tags.find(xuid);
		if (it != tags.end()) {
			if (del) {
				string result = it->second;
				tags.erase(it);
				return result;
			}
			else {
				return it->second;
			}
		}
		else {
			return "false";
		}
	}

	void pushNBT(string xuid, string N) {
		sNBT[xuid] = N;
	}
	string getNBT(string xuid, bool del = true) {
		std::map<string, string>::iterator it = sNBT.find(xuid);
		if (it != sNBT.end()) {
			if (del) {
				string result = it->second;
				sNBT.erase(it);
				return result;
			}
			else {
				return it->second;
			}
		}
		else {
			return "null";
		}
	}
} playerCache;

class PlayerTool {
public:
    //// Get ////
	static Player* getPlayerByXuid(string xuid) {
		
		auto playerList = Level::getAllPlayers();
		for (Player* p : playerList) {
			if (p->getXuid() == xuid) {
				return p;
			}
		}
		return nullptr;

		//Player* result = nullptr;
		//Global<Level>->forEachPlayer([&](Player& sp) -> bool {
		//	if (sp.getXuid() == xuid) {
		//		result = &sp;
		//		return true;
		//	}
		//	return false;
		//	});
		//return result;
	}
	static Player* getPlayerByRealName(string name) {
		auto playerList = Level::getAllPlayers();
		for (Player* p : playerList) {
			if (p->getRealName() == name) {
				return p;
			}
		}
		return nullptr;
	}
	static CompoundTag*   getBag       (Player* player) {
		try{
			CompoundTag* result = new CompoundTag();
			std::unique_ptr<CompoundTag> playerNBT = player->getNbt();
			result->put("Armor"    , playerNBT->get("Armor"    )->copy());
			result->put("Inventory", playerNBT->get("Inventory")->copy());
			result->put("Mainhand" , playerNBT->get("Mainhand" )->copy());
			result->put("Offhand"  , playerNBT->get("Offhand"  )->copy());
			return result;
		}
		catch (const std::exception&){
			logger.error("Error in PlayerTool::getBag");
			return {};
		}
	}
	static CompoundTag*   getEnderChest(Player* player) {
		try {
			CompoundTag* result = new CompoundTag();
			std::unique_ptr<CompoundTag> playerNBT = player->getNbt();
			result->put("EnderChestInventory", playerNBT->get("EnderChestInventory")->copy());
			return result;
		}
		catch (const std::exception&) {
			logger.error("Error in PlayerTool::getBag");
			return {};
		}
	}
	static CompoundTag*   getAttributes(Player* player) {
		try {
			CompoundTag* result = new CompoundTag();
			std::unique_ptr<CompoundTag> playerNBT = player->getNbt();
			result->put("Attributes", playerNBT->get("Attributes")->copy());
			return result;
		}
		catch (const std::exception&) {
			logger.error("Error in PlayerTool::getAttributes");
			return {};
		}
	}
	static CompoundTag*   getLevel     (Player* player) {
		try {
			CompoundTag* result = new CompoundTag();
			std::unique_ptr<CompoundTag> playerNBT = player->getNbt();
			result->put("PlayerLevel"        , playerNBT->get("PlayerLevel"        )->copy());
			result->put("PlayerLevelProgress", playerNBT->get("PlayerLevelProgress")->copy());
			return result;
		}
		catch (const std::exception&) {
			logger.error("Error in PlayerTool::getLevel");
			return {};
		}
	}
	static CompoundTag*   getTags      (Player* player) {
		try {
			CompoundTag* result = new CompoundTag();
			std::unique_ptr<CompoundTag> playerNBT = player->getNbt();
			result->put("Tags", playerNBT->get("Tags")->copy());
			return result;
		}
		catch (const std::exception&) {
			logger.error("Error in PlayerTool::getTags");
			return {};
		}
	}
	static nlohmann::json getScores    (Player* player) {
		std::vector<std::string> scoreList = ::Global<Scoreboard>->getObjectiveNames();
		nlohmann::json scores;
		for (int i = 0; i < scoreList.size(); i++) {
			scores.emplace(nlohmann::json{scoreList[i], Scoreboard::getScore(player, scoreList[i])});
		}
		return scores;
	}
	static money_t        getMoney     (Player* player) {
		if (syn_moneyType == "score") {
			return Scoreboard::getScore(player, syn_moneyName);
		}
		else if (syn_moneyType == "LLMoney") {
			
			return LLMoneyGet(player->getXuid());
		}
		else {
			logger.error("Failed to get player data: money. Invalid money type.(\'score\' or \'LLMoney\')");
			return -9961;
		}
	}
	static money_t        LLMoneyGet   (string xuid) {
		if (dynamicSymbolsMap.LLMoneyGet)
			return dynamicSymbolsMap.LLMoneyGet(xuid);
		else
		{
			logger.error("LLMoneyGet has not been loaded!");
			return 0;
		}
	}

	//// Set ////
	static bool setBag       (CompoundTag& playerNBT, CompoundTag& setNBT) {
		try
		{
			if (!&playerNBT || !&setNBT)
				return false;
			auto res = true;
			res = res && playerNBT.put("Armor"     , setNBT.get("Armor"    )->copy());
			res = res && playerNBT.put("Inventory" , setNBT.get("Inventory")->copy());
			res = res && playerNBT.put("Mainhand"  , setNBT.get("Mainhand" )->copy());
			res = res && playerNBT.put("Offhand"   , setNBT.get("Offhand"  )->copy());
			if(res) logger.debug("Set player data successfully: bag.");
			else    logger.warn ("Failed to set player data: bag.");
			return res;
		}
		catch (const std::exception&)
		{
			logger.error("Error in PlayerTool::setBag");
			return false;
		}
	}
	static bool setEnderChest(CompoundTag& playerNBT, CompoundTag& setNBT) {
		try
		{
			if (!&playerNBT || !&setNBT)
				return false;
			auto res = true;
			res = res && playerNBT.put("EnderChestInventory", setNBT.get("EnderChestInventory")->copy());
			logger.debug("Set player data successfully: enderChest.");
			return res;
		}
		catch (const std::exception&)
		{
			logger.error("Error in PlayerTool::setEnderChest");
			return false;
		}
	}
	static bool setAttributes(CompoundTag& playerNBT, CompoundTag& setNBT) {
		try
		{
			if (!&playerNBT || !&setNBT)
				return false;
			auto res = true;
			res = res && playerNBT.put("Attributes", setNBT.get("Attributes")->copy());
			logger.debug("Set player data successfully: attributes.");
			return res;
		}
		catch (const std::exception&)
		{
			logger.error("Error in PlayerTool::setAttributes");
			return false;
		}
	}
	static bool setLevel     (CompoundTag& playerNBT, CompoundTag& setNBT) {
		try
		{
			if (!&playerNBT || !&setNBT)
				return false;
			auto res = true;
			res = res && playerNBT.put("PlayerLevel"        , setNBT.get("PlayerLevel"        )->copy());
			res = res && playerNBT.put("PlayerLevelProgress", setNBT.get("PlayerLevelProgress")->copy());
			logger.debug("Set player data successfully: level.");
			return res;
		}
		catch (const std::exception&)
		{
			logger.error("Error in PlayerTool::setLevel");
			return false;
		}
	}
	static bool setTags      (CompoundTag& playerNBT, CompoundTag& setNBT) {
		try{
			if (!&playerNBT || !&setNBT)
				return false;
			auto res = true;
			res = res && playerNBT.put("Tags", setNBT.get("Tags")->copy());
			logger.debug("Set player data successfully: tags.");
			return res;
		}
		catch (const std::exception&){
			logger.error("Error in PlayerTool::setTags");
			return false;
		}
	}
	static void setScores    (Player* player, nlohmann::json scoreList) {
		for (auto score : scoreList.items()) {
			Scoreboard::setScore(player, score.key(), score.value());
		}
		logger.debug("Set player data successfully: scores.");
	}
	static bool setMoney     (Player* player, money_t money) {
		if (money != -9961) {
			if (syn_moneyType == "score") {
				if (Scoreboard::setScore(player, syn_moneyName, money)) {
					logger.debug("Set player data successfully: money.");
					return true;
				}
			}
			else if (syn_moneyType == "LLMoney") {
				if (LLMoneySet(player->getXuid(), money)){
					logger.debug("Set player data successfully: money.");
					return true;
				}
			}
			else {
				logger.error("Failed to set player data: money. Invalid money type.(\'score\' or \'LLMoney\')");
			}
		}
		return false;
	}
	static bool LLMoneySet   (string xuid, money_t money) {
		if (dynamicSymbolsMap.LLMoneySet)
			return dynamicSymbolsMap.LLMoneySet(xuid, money);
		else
		{
			logger.error("LLMoneySet has not been loaded!");
			return false;
		}
	}

	static bool setFromJson  (Player* player, nlohmann::json playerDataJson) {
		try {
			if (setBagVersion == 1) {
				std::unique_ptr<CompoundTag> playerTag = player->getNbt();
				if (syn_bag && playerDataJson["bag"] != "false") {
					std::unique_ptr<CompoundTag> data = CompoundTag::fromSNBT(playerDataJson["bag"]);
					PlayerTool::setBag(*playerTag, *data);
				}
				if (syn_enderChest && playerDataJson["enderChest"] != "false") {
					auto data = CompoundTag::fromSNBT(playerDataJson["enderChest"]);
					PlayerTool::setEnderChest(*playerTag, *data);
				}
				if (syn_attributes && playerDataJson["attributes"] != "false") {
					auto data = CompoundTag::fromSNBT(playerDataJson["attributes"]);
					PlayerTool::setAttributes(*playerTag, *data);
				}
				if (syn_level && playerDataJson["level"] != "false") {
					auto data = CompoundTag::fromSNBT(playerDataJson["level"]);
					PlayerTool::setLevel(*playerTag, *data);
				}
				if (syn_tags && playerDataJson["tags"] != "false") {
					auto data = CompoundTag::fromSNBT(playerDataJson["tags"]);
					PlayerTool::setTags(*playerTag, *data);
				}
				if (syn_scores && playerDataJson["scores"] != "false") {
					const string scoreString = playerDataJson["scores"];
					PlayerTool::setScores(player, json::parse(scoreString.c_str(), nullptr, true));
				}
				if (syn_money && playerDataJson["money"] != -9961) {
					playerCache.money[player->getXuid()] = playerDataJson["money"];
					// PlayerTool::setMoney(player, playerDataJson["money"]);
				}

				playerTag->setPlayer(player);
				// player->setNbt(playerTag.get());
			}
			else if (setBagVersion == 2) {
				std::unique_ptr<CompoundTag> playerTag = player->getNbt();
				if (syn_bag && playerDataJson["bag"] != "false") {
					std::unique_ptr<CompoundTag> data = CompoundTag::fromSNBT(playerDataJson["bag"]);
					PlayerTool::setBag(*playerTag, *data);
				}
				if (syn_enderChest && playerDataJson["enderChest"] != "false") {
					auto data = CompoundTag::fromSNBT(playerDataJson["enderChest"]);
					PlayerTool::setEnderChest(*playerTag, *data);
				}
				if (syn_attributes && playerDataJson["attributes"] != "false") {
					auto data = CompoundTag::fromSNBT(playerDataJson["attributes"]);
					PlayerTool::setAttributes(*playerTag, *data);
				}
				if (syn_level && playerDataJson["level"] != "false") {
					auto data = CompoundTag::fromSNBT(playerDataJson["level"]);
					PlayerTool::setLevel(*playerTag, *data);
				}
				if (syn_tags && playerDataJson["tags"] != "false") {
					auto data = CompoundTag::fromSNBT(playerDataJson["tags"]);
					PlayerTool::setTags(*playerTag, *data);
				}
				if (syn_scores && playerDataJson["scores"] != "false") {
					const string scoreString = playerDataJson["scores"];
					PlayerTool::setScores(player, json::parse(scoreString.c_str(), nullptr, true));
				}
				if (syn_money && playerDataJson["money"] != -9961) {
					playerCache.money[player->getXuid()] = playerDataJson["money"];
					// PlayerTool::setMoney(player, playerDataJson["money"]);
				}
				playerCache.pushNBT(player->getXuid(), playerTag->toSNBT());
				// playerTag->setPlayer(player);
				// player->setNbt(playerTag.get());
			}
			return true;
		}
		catch (const std::exception&) {
			logger.error("Error in PlayerTool::setFromJson");
			return false;
		}
	}
	
	//// Others ////
};