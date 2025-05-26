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

function getCelOffsetData(cel)
  if not cel then return nil end

  -- Only extract from cel's user data
  if cel.data and cel.data:match("offset:") then
    local x, y = cel.data:match("offset:(%d+),(%d+)")
    if x and y then
      return {
        offsetX = tonumber(x),
        offsetY = tonumber(y)
      }
    end
  end

  return nil
end

function setCelOffsetData(cel, offsetX, offsetY)
  if not cel then return end

  -- Store offset directly in the cel's user data only
  cel.data = string.format("offset:%d,%d", offsetX or 0, offsetY or 0)
  debug("Stored offset data in cel user data: " .. cel.data)
end

-- Check for offset tags
function spriteHasCelOffsetData(sprite)
  -- Check if any cel has offset data stored
  for i, layer in ipairs(sprite.layers) do
    local cel = layer:cel(1)
    if cel and cel.data and cel.data:match("offset:") then
      return true
    end
  end
  return false
end

-- Read offset data from a metadata file
function readFrameMetadata(metaPath)
  local offsetX, offsetY = 0, 0

  if app.fs.isFile(metaPath) then
    local meta = io.open(metaPath, "r")
    if meta then
      for line in meta:lines() do
        local key, value = line:match("(.+)=(.+)")
        if key == "offset_x" then offsetX = tonumber(value) end
        if key == "offset_y" then offsetY = tonumber(value) end
      end
      meta:close()
    end
  end

  return offsetX, offsetY
end

-- Scan frames and return dimensions and metadata
function scanFramesForDimensions(basePath)
  local maxWidth = 0
  local maxHeight = 0
  local maxOffsetX = 0
  local maxOffsetY = 0
  local maxRightEdge = -999999
  local maxBottomEdge = -999999
  local frameIndex = 0
  local frameData = {}

  while true do
    local framePath = basePath .. "_" .. frameIndex .. ".png"
    local metaPath = basePath .. "_" .. frameIndex .. ".meta"

    if not app.fs.isFile(framePath) then break end

    local image = Image{fromFile=framePath}
    local offsetX, offsetY = readFrameMetadata(metaPath)

    -- Store frame data
    frameData[frameIndex] = {
      path = framePath,
      width = image.width,
      height = image.height,
      offsetX = offsetX,
      offsetY = offsetY
    }

    -- Track maximums
    maxOffsetX = math.max(maxOffsetX, offsetX)
    maxOffsetY = math.max(maxOffsetY, offsetY)

    -- Track extents
    local rightEdge = offsetX + image.width
    local bottomEdge = offsetY + image.height
    maxRightEdge = math.max(maxRightEdge, rightEdge)
    maxBottomEdge = math.max(maxBottomEdge, bottomEdge)

    frameIndex = frameIndex + 1
  end

  return {
    frameCount = frameIndex,
    frameData = frameData,
    maxWidth = maxRightEdge,
    maxHeight = maxBottomEdge,
    maxOffsetX = maxOffsetX,
    maxOffsetY = maxOffsetY
  }
end

-- Position a cel based on offset
function positionCelWithOffset(cel, offsetX, offsetY, maxOffsetX, maxOffsetY)
  if not cel then return end

  -- Calculate position based on offset from top-left
  local adjustedX = maxOffsetX - offsetX
  local adjustedY = maxOffsetY - offsetY

  -- Set position
  cel.position = Point(adjustedX, adjustedY)

  -- Store offset data
  setCelOffsetData(cel, offsetX, offsetY)

  return adjustedX, adjustedY
end

-- Create and position a frame cel with proper metadata
function addFrameCel(sprite, layerIndex, frameImage, offsetX, offsetY, maxOffsetX, maxOffsetY)
  -- Create layer if needed (first layer already exists)
  local layer
  if layerIndex == 0 then
    layer = sprite.layers[1]
    layer.name = "Frame 0"
    -- Delete existing cel if present
    if layer:cel(1) then
      sprite:deleteCel(layer, 1)
    end
  else
    layer = sprite:newLayer()
    layer.name = "Frame " .. layerIndex
  end

  -- Calculate position
  local adjustedX = maxOffsetX - offsetX
  local adjustedY = maxOffsetY - offsetY

  -- Create new cel with correct position
  local cel = sprite:newCel(layer, 1, frameImage, Point(adjustedX, adjustedY))

  -- Store offset data
  setCelOffsetData(cel, offsetX, offsetY)

  debug("Positioned frame " .. layerIndex .. " at (" .. adjustedX .. "," .. adjustedY .. ") with dimensions " .. 
        frameImage.width .. "x" .. frameImage.height)
        
  return cel
end

function processImport(shpFile, outputBasePath, createSeparateFrames)
  if not converterExists then
    showError("SHP converter not found at: " .. converterPath)
    return false
  end

  debug("Importing SHP: " .. shpFile)
  debug("Output: " .. outputBasePath)

  -- Check if file exists
  if not app.fs.isFile(shpFile) then
    showError("SHP file not found: " .. shpFile)
    return false
  end

  -- Extract base filename from the SHP file (without path and extension)
  local shpBaseName = shpFile:match("([^/\\]+)%.[^.]*$") or "output"
  shpBaseName = shpBaseName:gsub("%.shp$", "")
  debug("Extracted SHP base name: " .. shpBaseName)

  -- Extract output directory from outputBasePath
  local outputDir = outputBasePath:match("(.*[/\\])") or ""

  -- The C++ converter creates files with pattern: outputPath_shpBaseName_number.png
  -- So if outputBasePath is "/tmp/output" and SHP is "avatar.shp", 
  -- it creates: "/tmp/output_avatar_0.png", "/tmp/output_avatar_1.png", etc.
  local outputBaseName = outputBasePath:match("([^/\\]+)$") or "output"
  local actualOutputBase = outputDir .. outputBaseName .. "_" .. shpBaseName
  debug("Expected output base: " .. actualOutputBase)

  -- Create command - simplified for import only
  local cmd = quoteIfNeeded(converterPath) .. " import " .. quoteIfNeeded(shpFile) .. " " .. quoteIfNeeded(outputBasePath)

  debug("Executing: " .. cmd)

  -- Execute command
  local success = executeHidden(cmd)

  -- Check for output files - look for the pattern the C++ converter actually creates
  local firstFrame = actualOutputBase .. "_0.png"

  debug("Looking for first frame at: " .. firstFrame)
  debug("File exists: " .. tostring(app.fs.isFile(firstFrame)))

  if not app.fs.isFile(firstFrame) then
    debug("ERROR: Failed to convert SHP file")
    return false
  end

  -- Continue with loading the frames using the correct output base
  debug("Loading output files into Aseprite")

  -- Scan all frames once to get dimensions and metadata
  local framesInfo = scanFramesForDimensions(actualOutputBase)
  
  if framesInfo.frameCount == 0 then
    debug("ERROR: No frames found after conversion")
    return false
  end
  
  debug("Found " .. framesInfo.frameCount .. " frames, canvas size: " 
        .. framesInfo.maxWidth .. "x" .. framesInfo.maxHeight)

  -- Open and setup the sprite
  debug("Opening first frame: " .. framesInfo.frameData[0].path)

  -- Disable animation detection before opening
  disableAnimationDetection()
  local sprite = app.open(framesInfo.frameData[0].path)
  restoreAnimationDetection()

  if not sprite then
    showError("Failed to open first frame")
    return false
  end

  -- Set filename
  sprite.filename = shpFile

  -- Save original first frame dimensions and image before resizing
  local frame0 = framesInfo.frameData[0]
  local originalImage = Image(sprite.cels[1].image)
  debug("Saved original first frame image: " .. originalImage.width .. "x" .. originalImage.height)

  -- Resize the sprite to the calculated dimensions
  if sprite.width ~= framesInfo.maxWidth or sprite.height ~= framesInfo.maxHeight then
    debug("Resizing from " .. sprite.width .. "x" .. sprite.height .. 
          " to " .. framesInfo.maxWidth .. "x" .. framesInfo.maxHeight)
    sprite:resize(framesInfo.maxWidth, framesInfo.maxHeight, 0, 0)
  end

  -- Rename the first layer
  local firstLayer = sprite.layers[1]
  firstLayer.name = "Frame 1"

  -- Replace the first cel with the original image
  -- or the offset is lost
  sprite:deleteCel(firstLayer, 1)

  -- Create a new cel with the original image
  local adjustedX = framesInfo.maxOffsetX - frame0.offsetX
  local adjustedY = framesInfo.maxOffsetY - frame0.offsetY
  local firstCel = sprite:newCel(firstLayer, 1, originalImage, Point(adjustedX, adjustedY))

  -- Store offset data
  setCelOffsetData(firstCel, frame0.offsetX, frame0.offsetY)

  debug("Positioned first cel at (" .. adjustedX .. "," .. adjustedY .. ") with dimensions " .. 
        originalImage.width .. "x" .. originalImage.height)

  -- Process each frame
  for frameIndex = 0, framesInfo.frameCount - 1 do
    local frame = framesInfo.frameData[frameIndex]
    local frameImage

    -- For first frame, use the saved original image
    if frameIndex == 0 then
      frameImage = originalImage
    else
      frameImage = Image{fromFile=frame.path}
    end

    -- Add as layer/cel with proper positioning
    addFrameCel(sprite, frameIndex, frameImage, frame.offsetX, frame.offsetY,
                framesInfo.maxOffsetX, framesInfo.maxOffsetY)
  end

  -- Set layer edges to be visible after import
  if app.preferences then
    -- Use the correct document preferences API
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

  return true, sprite
end

function importSHP(filename)
  -- Normal dialog flow for manual import
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