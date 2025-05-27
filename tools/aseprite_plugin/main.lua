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
local debugEnabled = true  -- toggle debug messages

local function debug(message)
  if debugEnabled then
    print("[Pentagram SHP] " .. message)
  end
end

local function logError(message)
  -- Always print errors regardless of debug setting
  print("[Pentagram SHP ERROR] " .. message)
end

-- Global utility function for quoting paths with spaces
function quoteIfNeeded(path)
  if path:find(" ") then
    return '"' .. path .. '"'
  else
    return path
  end
end

-- Helper to run commands with hidden output
function executeHidden(cmd)
  -- For debugging, run raw command instead with output captured
  if debugEnabled then
    debug("Executing with output capture: " .. cmd)
    local tmpFile = app.fs.joinPath(app.fs.tempPath, "pent-shp-output-" .. os.time() .. ".txt")

    -- Add output redirection to file
    local redirectCmd
    if app.fs.pathSeparator == "\\" then
      -- Windows
      redirectCmd = cmd .. " > " .. quoteIfNeeded(tmpFile) .. " 2>&1"
    else
      -- Unix-like (macOS, Linux)
      redirectCmd = cmd .. " > " .. quoteIfNeeded(tmpFile) .. " 2>&1"
    end

    -- Execute the command
    local success = os.execute(redirectCmd)

    -- Read and log the output
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
    -- Check operating system and add appropriate redirection
    local redirectCmd
    if app.fs.pathSeparator == "\\" then
      -- Windows
      redirectCmd = cmd .. " > NUL 2>&1"
    else
      -- Unix-like (macOS, Linux)
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

-- Verify converter exists at startup
local converterExists = app.fs.isFile(converterPath)
debug("Converter exists: " .. tostring(converterExists))

-- Check if converter is executable and only chmod if needed (Unix-like systems only)
-- Installing the plug-in might unset the executable bit
if converterExists and app.fs.pathSeparator == "/" then
  -- Check if a file is executable
  local function isExecutable(path)
    local cmd = "test -x " .. quoteIfNeeded(path)
    local result = os.execute(cmd)
    -- os.execute returns different values based on Lua version
    -- In Lua 5.2+, it returns success, exit_type, code
    -- In Lua 5.1, it returns the exit code
    if type(result) == "boolean" then
      return result
    else
      return result == 0
    end
  end

  -- Only chmod if the file exists but isn't executable
  if not isExecutable(converterPath) then
    debug("Converter found but not executable, setting permissions")
    local chmodCmd = "chmod +x " .. quoteIfNeeded(converterPath)
    executeHidden(chmodCmd)
  end
end

-- Error display helper
function showError(message)
  logError(message)
  app.alert{
    title="Pentagram SHP Error",
    text=message
  }
end

-- Don't detect Animation sequences when opening files
function disableAnimationDetection()
  -- Store the original preference value if it exists
  if app.preferences and app.preferences.open_file and app.preferences.open_file.open_sequence ~= nil then
    _G._originalSeqPref = app.preferences.open_file.open_sequence
    -- Set to 2 which means "skip the prompt without loading as animation"
    app.preferences.open_file.open_sequence = 2
  end
end

function restoreAnimationDetection()
  -- Restore the original preference if we saved it
  if app.preferences and app.preferences.open_file and _G._originalSeqPref ~= nil then
    app.preferences.open_file.open_sequence = _G._originalSeqPref
  end
end

-- File format registration function
function registerSHPFormat()
  if not converterExists then
    showError("SHP converter tool not found at:\n" .. converterPath .. 
              "\nSHP files cannot be opened until this is fixed.")
    return false
  end
  return true
end

-- Scan frames and return paths and count
function scanFrames(basePath)
  local frameIndex = 0
  local framePaths = {}

  while true do
    local framePath = basePath .. "_" .. frameIndex .. ".png"
    -- local metaPath = basePath .. "_" .. frameIndex .. ".meta" -- Meta not needed for dimensions anymore

    if not app.fs.isFile(framePath) then break end

    table.insert(framePaths, framePath)
    frameIndex = frameIndex + 1
  end

  return {
    frameCount = frameIndex,
    paths = framePaths
  }
end

-- Create and position a frame cel (always at 0,0 now)
function addFrameToSprite(sprite, layerIndex, frameImage)
  local layer
  if layerIndex == 0 then
    -- This is the first frame, which is already loaded when sprite was opened.
    -- We just ensure the layer name is set.
    layer = sprite.layers[1]
    layer.name = "Frame 1" -- Aseprite is 1-indexed for frames/layers in UI
    -- The cel is already there from app.open()
  else
    -- For subsequent frames, create a new layer and a new frame (cel)
    layer = sprite:newLayer()
    layer.name = "Frame " .. (layerIndex + 1) -- User-facing layer name
    -- Create new cel at frame (layerIndex + 1) because Aseprite frames are 1-indexed
    -- Cel position is (0,0) by default for new cels on new layers if image matches sprite size.
    -- If we are adding to existing layers, ensure we are on the correct Aseprite frame.
    -- For simplicity, assuming one layer per SHP frame, and one Aseprite frame.
    -- To put each SHP frame on a new Aseprite frame on the *same* layer:
    -- local cel = sprite:newCel(sprite.layers[1], layerIndex + 1, frameImage, Point(0,0))
    -- To put each SHP frame on a new *layer* (simplest for visual separation):
    sprite:newCel(layer, 1, frameImage, Point(0,0))
  end

  debug("Added/updated layer " .. layer.name .. " for SHP frame " .. layerIndex)
end

function processImport(shpFile, outputBasePath, createSeparateFrames) -- createSeparateFrames is not used anymore with this model
  if not converterExists then
    showError("SHP converter not found at: " .. converterPath)
    return false
  end

  debug("Importing SHP: " .. shpFile)
  debug("Output: " .. outputBasePath)

  if not app.fs.isFile(shpFile) then
    showError("SHP file not found: " .. shpFile)
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

  local success = executeHidden(cmd)

  local firstFramePath = actualOutputBase .. "_0.png"
  debug("Looking for first frame at: " .. firstFramePath)
  debug("File exists: " .. tostring(app.fs.isFile(firstFramePath)))

  if not app.fs.isFile(firstFramePath) then
    debug("ERROR: Failed to convert SHP file or first frame not found.")
    showError("Conversion failed or no output frames were generated.\nCheck console for C++ tool errors if debug is enabled.")
    return false
  end

  debug("Loading output files into Aseprite")

  local framesInfo = scanFrames(actualOutputBase)
  
  if framesInfo.frameCount == 0 then
    debug("ERROR: No frames found after conversion (scanFrames).")
    showError("No frames found after conversion, though first frame was present.")
    return false
  end
  
  debug("Found " .. framesInfo.frameCount .. " frames.")

  disableAnimationDetection()
  local sprite = app.open(framesInfo.paths[1]) -- Open the first frame
  restoreAnimationDetection()

  if not sprite then
    showError("Failed to open first frame: " .. framesInfo.paths[1])
    return false
  end

  sprite.filename = shpFile -- Set sprite filename to the original SHP
  
  -- First frame is already loaded, just name its layer.
  addFrameToSprite(sprite, 0, nil) -- Pass nil for image as it's already there

  -- Load subsequent frames onto new layers
  for frameIndex = 1, framesInfo.frameCount - 1 do
    local framePath = framesInfo.paths[frameIndex + 1] -- Lua tables are 1-indexed
    if app.fs.isFile(framePath) then
      local frameImage = Image{fromFile=framePath}
      if frameImage then
        if frameImage.width ~= sprite.width or frameImage.height ~= sprite.height then
            logError("Warning: Frame " .. frameIndex .. " (" .. framePath .. ") dimensions (" .. frameImage.width .. "x" .. frameImage.height .. ") do not match sprite dimensions (" .. sprite.width .. "x" .. sprite.height .. "). Skipping.")
        else
            addFrameToSprite(sprite, frameIndex, frameImage)
        end
      else
        logError("Failed to load image for frame: " .. framePath)
      end
    else
      logError("Frame PNG not found: " .. framePath)
    end
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

function importSHP(filename)
  local dlg = Dialog("Import U8 SHP File")
  dlg:file{
    id="shpFile",
    label="SHP File:",
    title="Select SHP File",
    open=true,
    filetypes={"shp"},
    focus=true
  }

  -- Store dialog result in outer scope
  local dialogResult = false
  local importSettings = {}

  dlg:button{
    id="import",
    text="Import",
    onclick=function()
      dialogResult = true
      importSettings.shpFile = dlg.data.shpFile
      dlg:close()
    end
  }

  dlg:button{
    id="cancel",
    text="Cancel",
    onclick=function()
      dialogResult = false
      dlg:close()
    end
  }

  -- Show dialog
  dlg:show()

  -- Handle result
  if not dialogResult then return end

  if not importSettings.shpFile or importSettings.shpFile == "" then
    showError("Please select an SHP file to import")
    return
  end

  -- Create temp directory for files
  local tempDir = app.fs.joinPath(app.fs.tempPath, "pent-shp-" .. os.time())
  app.fs.makeDirectory(tempDir)

  -- Prepare output file path
  local outputBasePath = app.fs.joinPath(tempDir, "output")
  
  return processImport(importSettings.shpFile, 
                      outputBasePath, 
                      true)
end

function init(plugin)
  debug("Initializing plugin...")

  -- Register file format first
  local formatRegistered = registerSHPFormat()
  debug("SHP format registered: " .. tostring(formatRegistered))

  -- Register import command only
  plugin:newCommand{
    id="ImportSHP",
    title="Import U8 SHP...",
    group="file_import",
    onclick=function() importSHP() end
  }
end

return { init=init }