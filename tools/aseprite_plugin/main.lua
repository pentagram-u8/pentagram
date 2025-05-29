--[[
 *
 *  main.lua - Aseprite plugin script for SHP files
 *
 *  Copyright (C) 2025  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ]]

local pluginName = "pent-shp"
local pluginDir = app.fs.joinPath(app.fs.userConfigPath, "extensions", pluginName)
local converterPath = app.fs.joinPath(pluginDir, "pent_shp")

-- Debug system with toggle
local debugEnabled = false  -- toggle debug messages

local function debug(message)
  if debugEnabled then
    print("[Pentagram SHP] " .. message)
  end
end

local function logError(message)
  -- Always print errors regardless of debug setting
  print("[Pentagram SHP ERROR] " .. message)
end

-- Utility function for quoting paths with spaces (now local)
local function quoteIfNeeded(path)
  if path:find(" ") then
    return '"' .. path .. '"'
  else
    return path
  end
end

-- Helper to run commands with hidden output (now local)
local function executeHidden(cmd)
  -- For debugging, run raw command instead with output captured
  if debugEnabled then
    debug("Executing with output capture: " .. cmd)
    local tmpFile = app.fs.joinPath(app.fs.tempPath, "pent-shp-output-" .. os.time() .. ".txt")
    local redirectCmd
    if app.fs.pathSeparator == "\\" then
      redirectCmd = cmd .. " > " .. quoteIfNeeded(tmpFile) .. " 2>&1"
    else
      redirectCmd = cmd .. " > " .. quoteIfNeeded(tmpFile) .. " 2>&1"
    end
    local success = os.execute(redirectCmd)
    if app.fs.isFile(tmpFile) then
      local file = io.open(tmpFile, "r")
      if file then
        debug("Command output:")
        local output = file:read("*all")
        debug(output or "<no output>")
        file:close()
      end
    end
    return success
  else
    local redirectCmd
    if app.fs.pathSeparator == "\\" then
      redirectCmd = cmd .. " > NUL 2>&1"
    else
      redirectCmd = cmd .. " > /dev/null 2>&1"
    end
    return os.execute(redirectCmd)
  end
end

debug("Plugin initializing...")
debug("Temp path: " .. app.fs.tempPath)
debug("User config path: " .. app.fs.userConfigPath)
debug("Converter expected at: " .. converterPath)

-- Check if converterPath exists with OS-specific extension
if not app.fs.isFile(converterPath) then
  debug("Converter not found, checking for extensions...")
  if app.fs.isFile(converterPath..".exe") then
    converterPath = converterPath..".exe"
    debug("Found Windows converter: " .. converterPath)
  elseif app.fs.isFile(converterPath..".bin") then
    converterPath = converterPath..".bin"
    debug("Found binary converter: " .. converterPath)
  end
end

-- Verify converter exists at startup (now local)
local converterExists = app.fs.isFile(converterPath)
debug("Converter exists: " .. tostring(converterExists))

-- Check if converter is executable (Unix-like systems only)
if converterExists and app.fs.pathSeparator == "/" then
  local function isExecutable(path) -- local to this block
    local cmd = "test -x " .. quoteIfNeeded(path)
    local result = os.execute(cmd)
    if type(result) == "boolean" then return result else return result == 0 end
  end
  if not isExecutable(converterPath) then
    debug("Converter found but not executable, setting permissions")
    local chmodCmd = "chmod +x " .. quoteIfNeeded(converterPath)
    executeHidden(chmodCmd) -- executeHidden is defined above
  end
end

-- Error display helper (now local)
local function showError(message)
  logError(message) -- logError is defined above
  app.alert{ title="Pentagram SHP Error", text=message }
end

-- Animation detection preferences (now local)
local function disableAnimationDetection()
  if app.preferences and app.preferences.open_file and app.preferences.open_file.open_sequence ~= nil then
    _G["_originalSeqPref_" .. pluginName] = app.preferences.open_file.open_sequence
    app.preferences.open_file.open_sequence = 2
  end
end

local function restoreAnimationDetection()
  if app.preferences and app.preferences.open_file and _G["_originalSeqPref_" .. pluginName] ~= nil then
    app.preferences.open_file.open_sequence = _G["_originalSeqPref_" .. pluginName]
    _G["_originalSeqPref_" .. pluginName] = nil
  end
end

-- File format registration function (now local)
local function registerSHPFormat()
  if not converterExists then -- converterExists is defined above
    showError("SHP converter tool not found at:\n" .. converterPath .. 
              "\nSHP files cannot be opened until this is fixed.")
    return false
  end
  return true
end

-- Scan frames and return paths and count (now local)
local function scanFrames(basePath)
  local frameIndex = 0
  local framePaths = {}
  while true do
    local framePath = basePath .. "_" .. frameIndex .. ".png"
    if not app.fs.isFile(framePath) then break end
    table.insert(framePaths, framePath)
    frameIndex = frameIndex + 1
  end
  return { frameCount = frameIndex, paths = framePaths }
end

-- Create and position a frame cel (now local)
local function addFrameToSprite(sprite, layerIndex, frameImage)
  local layer
  if layerIndex == 0 then
    layer = sprite.layers[1]
    layer.name = "Frame 1"
  else
    layer = sprite:newLayer()
    layer.name = "Frame " .. (layerIndex + 1)
    sprite:newCel(layer, 1, frameImage, Point(0,0))
  end
  debug("Added/updated layer " .. layer.name .. " for SHP frame " .. layerIndex)
end

-- Forward declaration for processImport as it's called by importPentagramSHP
local processImport

-- Main import function called by the command (already local)
local function importPentagramSHP() 
  local dlg = Dialog("Import U8 SHP File")
  dlg:file{ id="shpFile", label="SHP File:", title="Select SHP File", open=true, filetypes={"shp"}, focus=true }
  local dialogResult = false
  local importSettings = {}
  dlg:button{ id="import", text="Import", onclick=function()
      dialogResult = true
      importSettings.shpFile = dlg.data.shpFile
      dlg:close()
    end
  }
  dlg:button{ id="cancel", text="Cancel", onclick=function()
      dialogResult = false
      dlg:close()
    end
  }
  dlg:show()
  if not dialogResult then return end
  if not importSettings.shpFile or importSettings.shpFile == "" then
    showError("Please select an SHP file to import") -- showError is defined above
    return
  end
  local tempDir = app.fs.joinPath(app.fs.tempPath, "pent-shp-" .. os.time())
  app.fs.makeDirectory(tempDir)
  local outputBasePath = app.fs.joinPath(tempDir, "output")
  
  -- Call processImport (which will be defined below)
  return processImport(importSettings.shpFile, outputBasePath, true) 
end

-- Core import processing logic (now local)
processImport = function(shpFile, outputBasePath, createSeparateFrames)
  if not converterExists then
    showError("SHP converter not found at: " .. converterPath) -- showError is defined above
    return false
  end
  debug("Importing SHP: " .. shpFile)
  debug("Output: " .. outputBasePath)
  if not app.fs.isFile(shpFile) then
    showError("SHP file not found: " .. shpFile) -- showError is defined above
    return false
  end
  local shpBaseName = shpFile:match("([^/\\]+)%.[^.]*$") or "output"
  shpBaseName = shpBaseName:gsub("%.shp$", "")
  debug("Extracted SHP base name: " .. shpBaseName)
  local outputDir = outputBasePath:match("(.*[/\\])") or ""
  local outputBaseNameFromPath = outputBasePath:match("([^/\\]+)$") or "output"
  local actualOutputBase = outputDir .. outputBaseNameFromPath .. "_" .. shpBaseName
  debug("Expected output base: " .. actualOutputBase)
  local cmd = quoteIfNeeded(converterPath) .. " import " .. quoteIfNeeded(shpFile) .. " " .. quoteIfNeeded(outputBasePath)
  debug("Executing: " .. cmd)
  local success = executeHidden(cmd) -- executeHidden is defined above
  local firstFramePath = actualOutputBase .. "_0.png"
  debug("Looking for first frame at: " .. firstFramePath)
  debug("File exists: " .. tostring(app.fs.isFile(firstFramePath)))
  if not app.fs.isFile(firstFramePath) then
    debug("ERROR: Failed to convert SHP file or first frame not found.")
    showError("Conversion failed or no output frames were generated.\nCheck console for C++ tool errors if debug is enabled") -- showError is defined above
    return false
  end
  debug("Loading output files into Aseprite")
  local framesInfo = scanFrames(actualOutputBase) -- scanFrames is defined above
  if framesInfo.frameCount == 0 then
    debug("ERROR: No frames found after conversion (scanFrames).")
    showError("No frames found after conversion, though first frame was present.") -- showError is defined above
    return false
  end
  debug("Found " .. framesInfo.frameCount .. " frames.")
  disableAnimationDetection() -- defined above
  local sprite = app.open(framesInfo.paths[1])
  restoreAnimationDetection() -- defined above
  if not sprite then
    showError("Failed to open first frame: " .. framesInfo.paths[1]) -- showError is defined above
    return false
  end
  sprite.filename = shpFile
  addFrameToSprite(sprite, 0, nil) -- addFrameToSprite is defined above
  local finalTargetCanvasWidth = sprite.width
  local finalTargetCanvasHeight = sprite.height
  for frameIndex = 1, framesInfo.frameCount - 1 do
    local framePath = framesInfo.paths[frameIndex + 1]
    if app.fs.isFile(framePath) then
      local frameImage = Image{fromFile=framePath}
      if frameImage then
        if frameImage.width > finalTargetCanvasWidth then finalTargetCanvasWidth = frameImage.width end
        if frameImage.height > finalTargetCanvasHeight then finalTargetCanvasHeight = frameImage.height end
        addFrameToSprite(sprite, frameIndex, frameImage) -- addFrameToSprite is defined above
      else
        logError("Failed to load image for frame: " .. framePath) -- logError is defined above
      end
    else
      logError("Frame PNG not found: " .. framePath) -- logError is defined above
    end
  end
  local initialWidth = sprite.width
  local initialHeight = sprite.height
  if finalTargetCanvasWidth > initialWidth or finalTargetCanvasHeight > initialHeight then
    local addRight = 0
    if finalTargetCanvasWidth > initialWidth then addRight = finalTargetCanvasWidth - initialWidth end
    local addBottom = 0
    if finalTargetCanvasHeight > initialHeight then addBottom = finalTargetCanvasHeight - initialHeight end
    debug("Final canvas resize needed. Current: " .. initialWidth .. "x" .. initialHeight ..
          ", Target: " .. finalTargetCanvasWidth .. "x" .. finalTargetCanvasHeight ..
          ". Adding to borders: right=" .. addRight .. ", bottom=" .. addBottom)
    app.command.CanvasSize {
      ui = false, left = 0, top = 0, right = addRight, bottom = addBottom
    }
  else
    debug("No final canvas resize needed. Canvas size: " .. initialWidth .. "x" .. initialHeight .. 
          " (which is " .. finalTargetCanvasWidth .. "x" .. finalTargetCanvasHeight .. ")")
  end
  if app.preferences then
    local docPref = app.preferences.document(sprite)
    if docPref and docPref.show then
      docPref.show.layer_edges = true
      debug("Enabled layer edges display for this document")
    else
      debug("Could not set layer_edges preference (show section not found in document preferences)")
    end
  else
    debug("Could not set layer_edges preference (preferences not available)")
  end
  app.refresh()
  return true, sprite
end


-- Plugin initialization function
function init(plugin)
  debug("Initializing plugin (inside init function)...")
  local formatRegistered = registerSHPFormat()
  debug("SHP format registered (from init): " .. tostring(formatRegistered))
  
  debug("Attempting to register U8 SHP import command...")
  plugin:newCommand{
    id = pluginName .. "ImportSHP",
    title = "Import U8 SHP...",
    group = "file_import",
    onclick = function() 
      debug("Import SHP command clicked!")
      importPentagramSHP() 
    end
  }
  debug("U8 SHP Import command registration attempted in init")

end

return { init=init }