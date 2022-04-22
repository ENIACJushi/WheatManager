#pragma once
#include "Config.h"

class playerCache_ {
public:
	map<string, string> tags;            // xuid - tags
	map<string, int>    money;           // xuid - money
	std::list<string>   refusedPlayer;   // xuid, xuid...

	void pushRefusedPlayer(string xuid) {
		refusedPlayer.push_back(xuid);
	}
	bool findRefusedPlayer(string xuid, bool del = false) {
		for (std::list<string>::iterator id = playerCache.refusedPlayer.begin();
			id != refusedPlayer.end(); id++) {
			if (xuid == *id) {
				if (del) refusedPlayer.erase(id);
				return true;
			}
		}
		return false;
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
} playerCache;

static class PlayerTool {
public:
    //// Get ////
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
	static int            getMoney     (Player* player) {
		if (syn_moneyType == "score") {
			return Scoreboard::getScore(player, syn_moneyName);
		}
		else if (syn_moneyType == "LLMoney") {
			return LLMoneyGet(player->getXuid());
		}
		else {
			logger.error("Failed to get player data: money. Invalid money type.(\'score\' or \'LLMoney\')");
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
			logger.debug("Set player data successfully: bag.");
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
	static bool setMoney     (Player* player, long long int money) {
		if (money == -9961) {
			if (syn_moneyType == "score") {
				return Scoreboard::setScore(player, syn_moneyName, money);
				logger.debug("Set player data successfully: money.");
			}
			else if (syn_moneyType == "LLMoney") {
				return LLMoneySet(player->getXuid(), money);
				logger.debug("Set player data successfully: money.");
			}
			else {
				logger.error("Failed to set player data: money. Invalid money type.(\'score\' or \'LLMoney\')");
			}
		}
	}

	static bool setFromJson  (Player* player, nlohmann::json playerDataJson) {
		try {
			if (syn_bag        && playerDataJson["bag"]        != "false") {
				CompoundTag data;
				data.fromSNBT(playerDataJson["bag"]);
				PlayerTool::setBag(*player->getNbt(), data);
			}
			if (syn_enderChest && playerDataJson["enderChest"] != "false") {
				CompoundTag data;
				data.fromSNBT(playerDataJson["enderChest"]);
				PlayerTool::setEnderChest(*player->getNbt(), data);
			}
			if (syn_attributes && playerDataJson["attributes"] != "false") {
				CompoundTag data;
				data.fromSNBT(playerDataJson["attributes"]);
				PlayerTool::setAttributes(*player->getNbt(), data);
			}
			if (syn_level      && playerDataJson["level"]      != "false") {
				CompoundTag data;
				data.fromSNBT(playerDataJson["level"]);
				PlayerTool::setLevel(*player->getNbt(), data);
			}
			if (syn_tags       && playerDataJson["tags"]       != "false") {
				playerCache.tags[player->getXuid()] = playerDataJson["tags"];
				// CompoundTag data;
				// data.fromSNBT(playerDataJson["tags"]);
				// PlayerTool::setAttributes(*player->getNbt(), data);
			}
			if (syn_scores     && playerDataJson["scores"]     != "false") {
				PlayerTool::setScores(player, nlohmann::json(playerDataJson["scores"]));
			}
			if (syn_money      && playerDataJson["money"]      != -9961) {
				playerCache.money[player->getXuid()] = playerDataJson["money"];
				// PlayerTool::setMoney(player, playerDataJson["money"]);
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