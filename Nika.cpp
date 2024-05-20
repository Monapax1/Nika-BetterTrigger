#include "includes.hpp"

int main() {
    if (getuid()) { 
        std::cout << "RUN AS SUDO!\n"; 
        return -1; 
    }
    if (mem::GetPID() == 0) { 
        std::cout << "OPEN APEX LEGENDS!\n"; 
        return -1; 
    }

    // Initialisierung der Klassen mit Smart Pointern
    std::unique_ptr<ConfigLoader> cl(new ConfigLoader());
    std::unique_ptr<MyDisplay> display(new MyDisplay());
    std::unique_ptr<Level> level(new Level());
    std::unique_ptr<LocalPlayer> localPlayer(new LocalPlayer());
    std::unique_ptr<std::vector<Player*>> humanPlayers(new std::vector<Player*>);
    std::unique_ptr<std::vector<Player*>> dummyPlayers(new std::vector<Player*>);
    std::unique_ptr<std::vector<Player*>> players(new std::vector<Player*>);

    // Füllen der Spieler- und Dummy-Slots
    for (int i = 0; i < 60; i++) 
        humanPlayers->push_back(new Player(i, *localPlayer, *cl));
    for (int i = 0; i < 15000; i++) 
        dummyPlayers->push_back(new Player(i, *localPlayer, *cl));

    // Erstellung der Features mit Smart Pointern
    std::unique_ptr<NoRecoil> noRecoil(new NoRecoil(*cl, *display, *level, *localPlayer));
    std::unique_ptr<TriggerBot> triggerBot(new TriggerBot(*cl, *display, *localPlayer, *players));
    std::unique_ptr<Sense> sense(new Sense(*cl, *level, *localPlayer, *players));
    std::unique_ptr<Random> random(new Random(*cl, *display, *level, *localPlayer, *players));
    std::unique_ptr<Aim> aim(new Aim(*display, *localPlayer, *players, *cl));

    int counter = 0;
    
    while (true) {
        try {
            long startTime = util::currentEpochMillis();

            if (counter % 20 == 0) cl->reloadFile();
            level->readFromMemory();
            if (!level->playable) {
                printf("Player in Lobby - Sleep 35 sec\n");
                std::this_thread::sleep_for(std::chrono::seconds(35));
                continue;
            }

            localPlayer->readFromMemory();
            if (!localPlayer->isValid()) throw std::invalid_argument("Select Legend");

            // Spieler lesen
            players->clear();
            if (level->trainingArea)
                for (Player* p : *dummyPlayers) {
                    p->readFromMemory();
                    if (p->isValid()) players->push_back(p);
                }
            else
                for (Player* p : *humanPlayers) {
                    p->readFromMemory();
                    if (p->isValid()) players->push_back(p);
                }
                      
            noRecoil->controlWeapon();
            triggerBot->shootAtEnemy(counter);
            sense->update(counter);
            sense->itemGlow(counter);
            aim->update(counter);
            random->runAll(counter);
            
            int processingTime = static_cast<int>(util::currentEpochMillis() - startTime);
            int goalSleepTime = 6.97; // 16.67ms=60HZ | 6.97ms=144HZ
            int timeLeftToSleep = std::max(0, goalSleepTime - processingTime);
            std::this_thread::sleep_for(std::chrono::milliseconds(timeLeftToSleep));
            
            // Loop-Informationen gelegentlich ausgeben
            if (counter % 500 == 0)
                printf("| [%04d] - Time: %02dms |\n", counter, processingTime);
            
            // Zähler aktualisieren
            counter = (counter < 1000) ? ++counter : counter = 0;
        }
        catch (std::invalid_argument& e) {
            printf("[-] %s - SLEEP 5 SEC [-]\n", e.what());
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        catch (...) {
            printf("[-] UNKNOWN ERROR - SLEEP 3 SEC [-]\n");
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }    
    }
}
