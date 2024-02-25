/*
 * Copyright (c) 2010-2017 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "modulemanager.h"
#include "resourcemanager.h"

#include <framework/otml/otml.h>
#include <framework/core/application.h>
#include <list>

ModuleManager g_modules;

void ModuleManager::clear()
{
    m_modules.clear();
    m_autoLoadModules.clear();
}

void ModuleManager::discoverModules()
{
    // remove modules that are not loaded
    m_autoLoadModules.clear();

    // List of directories to search for modules
    std::vector<std::string> searchDirectories = {"/modules", "/mods"};

    // List of specific modules to load
    std::vector<std::string> specificModules = {
        "/modules/client/client.otmod",
        "/modules/client_background/background.otmod",
        "/modules/client_entergame/entergame.otmod",
        "/modules/client_feedback/feedback.otmod",
        "/modules/client_locales/locales.otmod",
        "/modules/client_mobile/mobile.otmod",
        "/modules/client_options/options.otmod",
        "/modules/client_profiles/profiles.otmod",
        "/modules/client_styles/styles.otmod",
        "/modules/client_textedit/textedit.otmod",
        "/modules/client_topmenu/topmenu.otmod",
        "/modules/corelib/corelib.otmod",
        "/modules/crash_reporter/crash_reporter.otmod",
        "/modules/game_actionbar/actionbar.otmod",
        "/modules/game_battle/battle.otmod",
        "/modules/game_bugreport/bugreport.otmod",
        "/modules/game_buttons/buttons.otmod",
        "/modules/game_console/console.otmod",
        "/modules/game_containers/containers.otmod",
        "/modules/game_cooldown/cooldown.otmod",
        "/modules/game_features/features.otmod",
        "/modules/game_healthinfo/healthinfo.otmod",
        "/modules/game_hotkeys/hotkeys_manager.otmod",
        "/modules/game_imbuing/imbuing.otmod",
        "/modules/game_interface/interface.otmod",
        "/modules/game_inventory/inventory.otmod",
        "/modules/game_itemselector/itemselector.otmod",
        "/modules/game_market/market.otmod",
        "/modules/game_minimap/minimap.otmod",
        "/modules/game_modaldialog/modaldialog.otmod",
        "/modules/game_npctrade/npctrade.otmod",
        "/modules/game_outfit/outfit.otmod",
        "/modules/client_terminal/terminal.otmod",
        "/modules/game_playerdeath/playerdeath.otmod",
        "/modules/game_playermount/playermount.otmod",
        "/modules/game_playertrade/playertrade.otmod",
        "/modules/game_prey/prey.otmod",
        "/modules/game_protocol/protocol.otmod",
        "/modules/game_questlog/questlog.otmod",
        "/modules/game_ruleviolation/ruleviolation.otmod",
        "/modules/game_shaders/shaders.otmod",
        "/modules/game_shop/shop.otmod",
        "/modules/game_skills/skills.otmod",
        "/modules/game_spelllist/spelllist.otmod",
        "/modules/game_stats/stats.otmod",
        "/modules/game_textmessage/textmessage.otmod",
        "/modules/game_textwindow/textwindow.otmod",
        "/modules/game_things/things.otmod",
        "/modules/game_topbar/topbar.otmod",
        "/modules/game_unjustifiedpoints/unjustifiedpoints.otmod",
        "/modules/game_viplist/viplist.otmod",
        "/modules/game_walking/walking.otmod",
        "/modules/gamelib/gamelib.otmod",
        "/modules/updater/updater.otmod",
        "/mods/game_healthbars/healthbars.otmod"
    };

    std::list<std::string> modules;

    // Iterate over each directory to find modules
    for (const auto& directory : searchDirectories) {
        auto dirs = g_resources.listDirectoryFiles(directory, true);
        for (const auto& dir : dirs) {
            auto subFilesAndDirs = g_resources.listDirectoryFiles(dir, true);
            modules.insert(modules.end(), subFilesAndDirs.begin(), subFilesAndDirs.end());
        }
    }

    // Iterate over each module found
    for (const auto& mod : modules) {
        // Check if any part of the module's name matches any part of the specific modules
        for (const auto& specificModule : specificModules) {
            if (mod.find(specificModule) != std::string::npos) {
                // If the module is of the correct file type, load it
                if (g_resources.isFileType(mod, "otmod")) {
                    ModulePtr module = discoverModule(mod);
                    if (module && module->isAutoLoad()) {
                        m_autoLoadModules.insert(std::make_pair(module->getAutoLoadPriority(), module));
                    }
                }
                break;
            }
        }
    }
}

void ModuleManager::autoLoadModules(int maxPriority)
{
    for(auto& pair : m_autoLoadModules) {
        int priority = pair.first;
        if(priority > maxPriority)
            break;
        ModulePtr module = pair.second;
        module->load();
    }
}

ModulePtr ModuleManager::discoverModule(const std::string& moduleFile)
{
    ModulePtr module;
    try {
        OTMLDocumentPtr doc = OTMLDocument::parse(moduleFile);
        OTMLNodePtr moduleNode = doc->at("Module");

        std::string name = moduleNode->valueAt("name");

        bool push = false;
        module = getModule(name);
        if(!module) {
            module = ModulePtr(new Module(name));
            push = true;
        }
        module->discover(moduleNode);

        // not loaded modules are always in back
        if(push)
            m_modules.push_back(module);
    } catch(stdext::exception& e) {
        g_logger.error(stdext::format("Unable to discover module from file '%s': %s", moduleFile, e.what()));
    }
    return module;
}

void ModuleManager::ensureModuleLoaded(const std::string& moduleName)
{
    ModulePtr module = g_modules.getModule(moduleName);
    if(!module || !module->load())
        g_logger.fatal(stdext::format("Unable to load '%s' module", moduleName));
}

void ModuleManager::unloadModules()
{
    auto modulesBackup = m_modules;
    for(const ModulePtr& module : modulesBackup)
        module->unload();
}

void ModuleManager::reloadModules()
{
    std::deque<ModulePtr> toLoadList;

    // unload in the reverse direction, try to unload upto 10 times (because of dependencies)
    for(int i=0;i<10;++i) {
        auto modulesBackup = m_modules;
        for(const ModulePtr& module : modulesBackup) {
            if(module->isLoaded() && module->canUnload()) {
                module->unload();
                toLoadList.push_front(module);
            }
        }
    }

    for(const ModulePtr& module : toLoadList)
        module->load();
}

ModulePtr ModuleManager::getModule(const std::string& moduleName)
{
    for(const ModulePtr& module : m_modules)
        if(module->getName() == moduleName)
            return module;
    return nullptr;
}

void ModuleManager::updateModuleLoadOrder(ModulePtr module)
{
    auto it = std::find(m_modules.begin(), m_modules.end(), module);
    if(it != m_modules.end())
        m_modules.erase(it);
    if(module->isLoaded())
        m_modules.push_front(module);
    else
        m_modules.push_back(module);
}
