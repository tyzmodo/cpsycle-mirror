-- psycle callstack (c) 2015 by psycledelics
-- File: callstack.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local listview = require("psycle.ui.canvas.listview")
local group = require("psycle.ui.canvas.group")
local rect = require("psycle.ui.canvas.rect")
local text = require("psycle.ui.canvas.text")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()

local callstack = listview:new()

function callstack:new(parent, listener)
  local c = listview:new(parent)  
  setmetatable(c, self)
  self.__index = self  
  c:init()
  c.listener_ = listener
  return c
end

function callstack:init()   
  self:addcolumn("", 50)
      :addcolumn("Name", 400)  
      :addcolumn("Source", 200)  
      :setautosize(false, false)  
end

--function callstack:add(info)  
--  local Index = self:inserttext(self.row, "")
--  self:settext(Index, 1, info.name.." Line "..info.line)  
--  self:settext(Index, 2, info.source:match("([^\\]+)$"))
--  self.row = self.row + 1  
--end

--[[function onrowclick(self)
  self.that.listener_:oncallstackclick(self.info) 
  local index = self:parent():itemindex(self)
  self.that:setdepth(index-1)  
end

function callstack:add(info)  
  local Index = self:inserttext(self.row, "")
  self:settext(Index, 1, info.name.." Line "..info.line)  
  self:settext(Index, 2, info.source:match("([^\\]+)$"))
  self.row = self.row + 1  
end

function callstack:setdepth(depth)
end
--]]

return callstack