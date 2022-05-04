#pragma once
#include <HookAPI.h>
#include <EventAPI.h>
#include <MC/NetworkIdentifier.hpp>
#include <MC/TextPacket.hpp>
#include <MC/TextPacket.hpp>
#include <GlobalServiceAPI.h>
#include <Global.h>
#include <MC/ReadOnlyBinaryStream.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/ChatEvent.hpp>
#include <MC/ColorFormat.hpp>
#include <MC/LevelStorage.hpp>
#include "Config.h"
#include "PlayerTool.h"

// 因无法解析textpacket而没有使用
#ifdef testServerNetworkHandler.handle
TInstanceHook(void, "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVTextPacket@@@Z",
    ServerNetworkHandler, NetworkIdentifier* n, TextPacket* t) {
    logger.info("packet");
    ServerPlayer* p = t->getPlayerFromPacket(Global<ServerNetworkHandler>, n);

    ReadOnlyBinaryStream* stream = new ReadOnlyBinaryStream("");
    ReadOnlyBinaryStream* stream1 = new ReadOnlyBinaryStream("", true);
    ReadOnlyBinaryStream* stream2 = new ReadOnlyBinaryStream("", false);
    t->_read(*stream);
    t->_read(*stream1);
    t->_read(*stream2);
    // t->readExtended(*stream);
    // t->readExtended(*stream1);
    // t->readExtended(*stream2);

    // getString getData无结果
    // getReadPointer 0
    std::cout << "player:\t" << p->getName()
        << "\nname:\t"       << t->getName()
        << "\nid:\t"         << (int)t->getId()
        << "\nstream:\t"     << stream->getLength()
        << "\nstream1:\t"    << stream1->getLength()
        << "\nstream2:\t"    << stream2->getLength()
        << "\n";

    return original(this, n, t);
};
#endif

// 拦截加入消息
THook(void, "?onReady_ClientGeneration@ServerNetworkHandler@@QEAAXAEAVPlayer@@AEBVNetworkIdentifier@@@Z", ServerNetworkHandler* a1,
    Player* a2,
    const struct NetworkIdentifier* a3) {
    if (playerCache.findTransformingPlayer(a2->getRealName(), true)) {
        return;
    }
    else {
        webAPI::broadcastMessage_join(a2->getRealName());
    }
    return original(a1, a2, a3);
}

// 拦截退出消息
THook(void, "?_onPlayerLeft@ServerNetworkHandler@@AEAAXPEAVServerPlayer@@_N@Z", ServerNetworkHandler* a1, ServerPlayer* a2, char a3) {
    if (playerCache.findTransformingPlayer(a2->getRealName(), true)) {
        auto cert = a2->getCertificate();
        if (cert) {
            auto& UniqueID = a2->getUniqueID();
            Global<LevelStorage>->save(*a2);
            a2->disconnect();
            a2->remove();
        }
        return;
    }
    else {
        webAPI::broadcastMessage_left(a2->getRealName());
    }
    return original(a1, a2, a3);
}

// 拦截translate消息
THook(void, "?createTranslated@TextPacket@@SA?AV1@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@3@@Z",
    void* self, string& s, std::vector<string>& v) {
    // 加入消息
    if (s == (ColorFormat::YELLOW + "%multiplayer.player.joined")) {
    }
    // 退出消息
    if (s == (ColorFormat::YELLOW + "%multiplayer.player.left")) {
    }
    original(self, s, v);
}