--[[ 
psycle pluginselector (c) 2016 by psycledelics
File: pluginselector.lua
copyright 2016 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.
]]

local systems = require("psycle.ui.systems")
local cfg = require("psycle.config"):new("PatternVisual")
local node = require("psycle.node")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local fontinfo = require("psycle.ui.fontinfo")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local lexer = require("psycle.ui.lexer")
local scintilla = require("psycle.ui.scintilla")
local settings = require("settings")
local search = require("search")
local serpent = require("psycle.serpent")

local textpage = scintilla:new()

textpage.windowtype = 95
textpage.pagecounter = 0

function textpage:new(parent)    
  local c = scintilla:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  textpage.pagecounter = textpage.pagecounter + 1
  systems:new():changewindowtype(textpage.windowtype, c)
  return c
end

function textpage:init()
  self:setautosize(false, false)
end

function textpage:onkeydown(ev)
  if ev:ctrlkey() then
    if ev:keycode() == 70 or ev:keycode() == 83 then
      ev:preventdefault()
    end            
  end
end    

function textpage:onmarginclick(linepos)
  self:addbreakpoint(linepos)
end

function textpage:addbreakpoint(linepos)
  self:definemarker(1, 31, 0x0000FF, 0x0000FF)
  self:addmarker(linepos, 1)
end

function textpage:status()  
  local that = self
  return {
    line = that:line() + 1,
    column = that:column() + 1,
	  modified = that:modified(),
	  ovrtype = that:ovrtype()
  }  
end

function textpage:createdefaultname()
  return "new"..textpage.pagecounter  
end

function textpage:findprevsearch(page, pos)
  local cpmin, cpmax = 0, 0
  cpmin = pos
  cpmax = 0
  if self:hasselection() then 
    cpmax = cpmax - 1
  end
  return cpmin, cpmax
end

function textpage:findnextsearch(page, pos)
  local cpmin, cpmax = 0, 0
  cpmin = pos
  cpmax = self:length()
  if self:hasselection() then       
    cpmin = cpmin + 1
  end
  return cpmin, cpmax
end

function textpage:findsearch(page, dir, pos)
  local cpmin, cpmax = 0, 0
  if dir == search.DOWN then      
    cpmin, cpmax = self:findnextsearch(self, pos)
  else
    cpmin, cpmax = self:findprevsearch(self, pos)
  end
  return cpmin, cpmax
end

function textpage:onsearch(searchtext, dir, case, wholeword, regexp)    
  self:setfindmatchcase(case):setfindwholeword(wholeword):setfindregexp(regexp)      
  local cpmin, cpmax = self:findsearch(self, dir, self:selectionstart())
  local line, cpselstart, cpselend = self:findtext(searchtext, cpmin, cpmax)
  if line == -1 then      
    if dir == search.DOWN then
	  self:setsel(0, 0)
	  local cpmin, cpmax = self:findsearch(self, dir, 0)
	  line, cpselstart, cpselend = self:findtext(searchtext, cpmin, cpmax)        
	else
	  self:setsel(0, 0)
	  local cpmin, cpmax = self:findsearch(self, dir, self:length())
	  line, cpselstart, cpselend = self:findtext(searchtext, cpmin, cpmax)
	end             
  end
  if line ~= -1 then
	self:setsel(cpselstart, cpselend)
	if self.searchbeginpos == cpselstart then
	  self.searchbegin = -1        
	  self.searchrestart = true
	else
	  self.searchrestart = false
	end
	if self.searchbeginpos == -1 then
	  self.searchbeginpos = cpselstart        
	end
  end      
end


function textpage:setproperties(properties)   
  if properties.color then    
    self:setforegroundcolor(properties.color:value())	    
  end
  if properties.backgroundcolor then    
	  self:setbackgroundcolor(properties.backgroundcolor:value())
  end
  self:styleclearall()
  self:showcaretline()    
  local lexsetters = {"commentcolor", "commentlinecolor", "commentdoccolor",
                      "foldingmarkerforecolor", "foldingmarkerbackcolor",
                      "operatorcolor", "wordcolor", "stringcolor",
                      "identifiercolor", "numbercolor"}  
  local lex = lexer:new()  
  for _, setter in pairs(lexsetters) do        
    local property = properties[setter]      
    if property then            
      lex["set"..setter](lex, property:value())
    end
  end 
  self:setlexer(lex)  
  local setters = {"linenumberforegroundcolor", "linenumberbackgroundcolor", "foldingbackgroundcolor",
                   "selbackgroundcolor",
                   "caretcolor", "caretlinebackgroundcolor"} 
  for _, setter in pairs(setters) do        
    local property = properties[setter]      
    if property then            
      textpage["set"..setter](self, property:value())
    end
  end    
  self:setselalpha(75)    
  self:showcaretline()  
end

return textpage