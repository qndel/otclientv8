extraHotkeys = {}
local previousCreature = nil

function attackCreatureTile(tmppos)
	local CreatureButtonColors = {
		onIdle = {
			notHovered = '#888888',
			hovered = '#FFFFFF'
		},
		onTargeted = {
			notHovered = '#FF0000',
			hovered = '#FF8888'
		},
		onFollowed = {
			notHovered = '#00FF00',
			hovered = '#88FF88'
		}
	}
	local tile = g_map.getTile(tmppos)
	if tile ~= nil then
		local creature = tile:getTopCreature()
		if creature ~= nil then
			if creature:isMonster() == false then
				return false
			end
			if creature:getHealthPercent() <= 0 then
				return false
			end
			if previousCreature ~= nil then
				--local player = g_game.getLocalPlayer()
				--local pos = player:getPosition()
				--if previousCreature:getPosition() ~= nil and isAdjacent(pos, previousCreature:getPosition()) == true then
				--	return true
				--end
				previousCreature:hideStaticSquare()
			end
			previousCreature = creature
			g_game.attack(creature)
			creature:showStaticSquare('#A020F0')
			--if attackedCreatureIdMap[creature:getId()] == nil then
			--	attackedCreatureIdMap[creature:getId()] = os.time()
			--	connect(creature, { onDeath = attackTargetDied })
			--end
			return true
		end
	end
	return false
end

function attackNearest(param_maxRange)
	local player = g_game.getLocalPlayer()
	local pos = player:getPosition()
	local maxRange = 2
	if param_maxRange ~= nil then
		maxRange = param_maxRange
	end
	--maxRange = 2
	-- first scan adjacent tile for any wounded enemy - prioritize that
	for xx=-1,1 do
		for yy=-1,1 do
			if math.abs(xx) == 1 or math.abs(yy) == 1 then
				local tmppos = {x=pos.x + xx, y=pos.y + yy,z=pos.z}
				local tile = g_map.getTile(tmppos)
				if tile ~= nil then
					local creature = tile:getTopCreature()
					if creature ~= nil then
						if creature:isMonster() == true then
							--printWithTimestamp("enemy health %: " .. tonumber(creature:getHealthPercent()))
							if creature:getHealthPercent() < 100 then
								if attackCreatureTile(tmppos) == true then
									return true
								end							
							end
						end
					end
				end
			end
		end
	end
	for radius=1,maxRange do
		for xx=-radius,radius do
			for yy=-radius,radius do
				if math.abs(xx) == radius or math.abs(yy) == radius then
					local tmppos = {x=pos.x + xx, y=pos.y + yy,z=pos.z}
					if attackCreatureTile(tmppos) == true then
						--printWithTimestamp("attack nearest - return true")
						return true
					end
				end
			end
		end
	end
	--printWithTimestamp("attack nearest - return false")
	return false
end


function addExtraHotkey(name, description, callback)
  table.insert(extraHotkeys, {
    name = name:lower(),
    description = tr(description),
    callback = callback
  })
  
end

function setupExtraHotkeys(combobox)
  addExtraHotkey("none", "None", nil)
  addExtraHotkey("cancelAttack", "Stop attacking", function(repeated)
    if not repeated then
      g_game.attack(nil)
    end
  end)
  addExtraHotkey("attackNearest", "Attack nearest", function(repeated)
    if not repeated then
      attackNearest(7)
    end
  end)
  addExtraHotkey("attackNext", "Attack next target from battle list", function(repeated)
    if repeated or not modules.game_battle then
      return
    end
    local battlePanel = modules.game_battle.battlePanel
    local attackedCreature = g_game.getAttackingCreature()
    local nextChild = nil
    local breakNext = false
    for i, child in ipairs(battlePanel:getChildren()) do    
      if not child.creature or not child:isOn() then
        break
      end
      nextChild = child
      if breakNext then
        break
      end
      if child.creature == attackedCreature then
        breakNext = true
        nextChild = battlePanel:getFirstChild()
      end
    end
    if not breakNext then
      nextChild = battlePanel:getFirstChild()
    end
    if nextChild and nextChild.creature ~= attackedCreature then
      g_game.attack(nextChild.creature)
    end
  end)
  
  addExtraHotkey("attackPrevious", "Attack previous target from battle list", function(repeated)
    if repeated or not modules.game_battle then
      return
    end
    local battlePanel = modules.game_battle.battlePanel
    local attackedCreature = g_game.getAttackingCreature()
    local prevChild = nil
    for i, child in ipairs(battlePanel:getChildren()) do
      if not child.creature or not child:isOn() then
        break
      end
      if child.creature == attackedCreature then
        break
      end
      prevChild = child    
    end
    if prevChild and prevChild.creature ~= attackedCreature then
      g_game.attack(prevChild.creature)
    end
  end)

  addExtraHotkey("toggleWsad", "Enable/disable wsad walking", function(repeated)
    if repeated or not modules.game_console then
      return
    end
    if not modules.game_console.consoleToggleChat:isChecked() then
      modules.game_console.disableChat(true) 
    else
      modules.game_console.enableChat(true) 
    end    
  end)  
  
  
  
  
  
  
  
  
  
  
  
  

  
  for index, actionDetails in ipairs(extraHotkeys) do
    combobox:addOption(actionDetails.description)
  end
end

function executeExtraHotkey(action, repeated)
  action = action:lower()
  for index, actionDetails in ipairs(extraHotkeys) do
    if actionDetails.name == action and actionDetails.callback then
      actionDetails.callback(repeated)
    end
  end
end

function translateActionToActionComboboxIndex(action)
  action = action:lower()
  for index, actionDetails in ipairs(extraHotkeys) do
    if actionDetails.name == action then
      return index
    end
  end
  return 1
end

function translateActionComboboxIndexToAction(index)
  if index > 1 and index <= #extraHotkeys then
    return extraHotkeys[index].name  
  end
  return nil
end

function getActionDescription(action)
  action = action:lower()
  for index, actionDetails in ipairs(extraHotkeys) do
    if actionDetails.name == action then
      return actionDetails.description
    end
  end
  return "invalid action"
end