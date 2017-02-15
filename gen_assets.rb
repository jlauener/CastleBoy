#!/usr/bin/env ruby
require 'rubygems'
require 'json'
require 'chunky_png'

#
# ---------------------- asset file ------------------------------------------------------
#

class AssetFile
  attr :name
  attr :files

  def initialize(name)
    @name = name
    @files = []
  end

  def filename
    "#{name}.h"
  end

  def to_define_name
    name.upcase + "_H"
  end

  def to_s
    header = "#ifndef #{to_define_name}\n#define #{to_define_name}\n\n"
    footer = "#endif\n"
    core = @files.map(&:to_s).join("")
    header + core + footer
  end
end

assets = AssetFile.new("assets")

#
# ---------------------- map2ard ------------------------------------------------------
#

class MapData
  MAX_WIDTH = 256
  MAX_HEIGHT = 8
  MAX_ENTITIES = 32
  PER_LINE = 8
    
  TILED_TILE_ID_MIN = 0
  TILED_TILE_ID_MAX = 3
  TILED_ENTITY_ID_MIN = 9
  TILED_ENTITY_ID_MAX = 25
  TILED_PLAYER_ID = 25  
  TILED_FALLING_PLATFORM_ID = 9
  TILED_MOVING_PLATFORM_RIGHT_ID = 10
  TILED_MOVING_PLATFORM_LEFT_ID = 11
  
  TILE_EMPTY = 0
  TILE_PROP = 1
  TILE_MISC = 2
  TILE_GROUND = 3
  MAX_RLE_TILE = [64, 1, 4, 64]
  RLE_SIZE = [6, 0, 2, 6]  
  
  ENTITY_SKULL = 8
  ENTITY_SKULL_X_OFFSET = 7
  
  TYPE_INDOOR_NAME = "indoor"
  TYPE_INDOOR_VALUE = 0x8
  
  TYPE_OUTDOOR_NAME = "outdoor"
  TYPE_OUTDOOR_VALUE = 0x4
  
  TYPE_CAVE_NAME = "cave"
  TYPE_CAVE_VALUE = 0x0
  
  TYPE_BOSS_NAME = "boss"
  TYPE_BOSS_VALUE = 0xC

  attr :name  
  attr :width
  attr :height 
  attr :playerX
  attr :playerY
  attr :entityCount
  attr :tileMap
  attr :entityMap
  attr :size  
  attr :tileDataSize
  attr :tileDataSizeRLE
  attr :mapType  
  attr :mapTypeStr

  def initialize(fileName)    
    jsonHash = JSON.parse(File.read(fileName))
    @name = File.basename(fileName,".*")
    @width = jsonHash["width"]
    @height = jsonHash["height"]    
    @size = 0
    if @width > MAX_WIDTH
      raise "Map #{@name} is too wide, max is #{MAX_WIDTH}, current is #{@width}."
    end

    if @height > MAX_HEIGHT
      raise "Map #{@name} is too high, max is #{MAX_HEIGHT}, current is #{@height}."
    end
    
    @tileDataSize = (@width * @height) / 4
    
    backgroundData = jsonHash["layers"][0]["data"]
    mainData = jsonHash["layers"][1]["data"]      
    
    # find player start
    playerFound = false
    for iy in 0...@height
      for ix in 0...@width        
        if mainData[iy * @width + ix] == TILED_PLAYER_ID
          @playerX = ix
          @playerY = iy          
          playerFound = true;
          mainData[iy * @width + ix] = 0                    
        end
      end
    end
    if !playerFound
      raise "Map #{@name} has no player starting position."
    end
    
  #merge falling and moving platforms
    for iy in 0...@height
      for ix in 0...@width
        if mainData[iy * @width + ix] == TILED_FALLING_PLATFORM_ID
          if mainData[iy * @width + ix + 1] != TILED_FALLING_PLATFORM_ID
              raise "Map #{@name} has invalid falling platform at #{ix},#{iy}. 2 expected"
          end
          mainData[iy * @width + ix + 1] = 0
        elsif mainData[iy * @width + ix] == TILED_MOVING_PLATFORM_LEFT_ID
          if mainData[iy * @width + ix + 1] != TILED_MOVING_PLATFORM_LEFT_ID or mainData[iy * @width + ix + 2] != TILED_MOVING_PLATFORM_LEFT_ID
              raise "Map #{@name} has invalid moving platform at #{ix},#{iy}. 3 expected"
          end
          mainData[iy * @width + ix + 1] = 0
          mainData[iy * @width + ix + 2] = 0
        elsif mainData[iy * @width + ix] == TILED_MOVING_PLATFORM_RIGHT_ID
          if mainData[iy * @width + ix + 1] != TILED_MOVING_PLATFORM_RIGHT_ID or mainData[iy * @width + ix + 2] != TILED_MOVING_PLATFORM_RIGHT_ID
              raise "Map #{@name} has invalid moving platform at #{ix},#{iy}. 3 expected"
          end
          mainData[iy * @width + ix + 1] = 0
          mainData[iy * @width + ix + 2] = 0
        end
      end
    end

    # extract tile and entity map
    @tileMap = []
    @entityCount = 0
    @entityMap = []
    for iy in 0...@height
      for ix in 0...@width
        id = mainData[iy * @width + ix]      
        if id < TILED_TILE_ID_MIN
          raise "Map #{@name} has invalid tile ID #{id} in main layer at #{ix},#{iy}."
        elsif id == 0
          @tileMap[iy * @width + ix] = 0
          @entityMap[iy * @width + ix] = -1        
        elsif id <= TILED_TILE_ID_MAX
          @tileMap[iy * @width + ix] = id
          @entityMap[iy * @width + ix] = -1
        elsif id >= TILED_ENTITY_ID_MIN and id <= TILED_ENTITY_ID_MAX
          @tileMap[iy * @width + ix] = 0
          @entityMap[iy * @width + ix] = id - TILED_ENTITY_ID_MIN
		  @entityCount += 1
        else
          raise "Map #{@name} has invalid tile ID #{id} in main layer at #{ix},#{iy}."
        end
        
        if @tileMap[iy * @width + ix] == 0
          id = backgroundData[iy * @width + ix]      
          if id < TILED_TILE_ID_MIN or id > TILED_TILE_ID_MAX
            raise "Map #{@name} has invalid tile ID #{id} in prop layer at #{ix},#{iy}."
          else
            @tileMap[iy * @width + ix] = id
          end
        end
      end
    end
	if @entityCount > MAX_ENTITIES
      raise "Map #{@name} has too many entities, max is #{MAX_ENTITIES}, current is #{@entityCount}."
    end
    
	# extract map type (indoor, outdoor, ...)
    @mapTypeStr = jsonHash["properties"]["type"]
    if @mapTypeStr == TYPE_INDOOR_NAME
      @mapType = TYPE_INDOOR_VALUE
    elsif @mapTypeStr == TYPE_OUTDOOR_NAME
      @mapType = TYPE_OUTDOOR_VALUE
    elsif @mapTypeStr == TYPE_CAVE_NAME
      @mapType = TYPE_CAVE_VALUE
    elsif @mapTypeStr == TYPE_BOSS_NAME
      @mapType = TYPE_BOSS_VALUE
    else
      raise "Map #{@name} has an invalid type '#{@mapTypeStr}'."
    end
    
    simulateRLE()
  end  
  
  def code_header
    o = "// name: #{@name}\n"
    o << "// type: #{@mapTypeStr}\n"
    o << "// size: #{@width}x#{@height}\n"
    o << "//\n"
    @tileMap.each_with_index do |tile, i|
      o << "// " if i % @width == 0
      o << tile.to_s
      o << "\n" if (i + 1) % @width == 0
    end
    o << "\n"
    o
  end
  
  def add_byte(o, byte)
    hex = byte.to_s(16).upcase
    hex = "0x" + (hex.length == 1 ? "0" : "") + hex + ","
    o << hex
    @size += 1
    self
  end
  
  def meta_data
    o = ""    
    add_byte(o, @playerY + (@mapType << 4))
    add_byte(o, @playerX)          
    o << "\n"
    o
  end
  
  def tilemap_data
    o = ""
    total = 0
    byte = 0
    count = 0
    for ix in 0...@width
      for iy in 0...@height
        tile = @tileMap[iy * @width + ix]
        byte += tile << count;        
        count += 2

        if count == 8
          add_byte(o, byte)
          total += 1
          o << "\n" if total % PER_LINE == 0
          count = 0
          byte = 0
        end
      end
    end

    if count != 0
      raise "Map #{@name} has wrong size, must be a multiple of 8."
    end
    o << "\n"
    o
  end
  
  def entity_data
    o = ""
    total = 0
    for ix in 0...@width
      for iy in 0...@height
        entity = @entityMap[iy * @width + ix]
        if entity >= 0
          x = ix
          y = iy
          if entity == ENTITY_SKULL
            x += ENTITY_SKULL_X_OFFSET
          end
          add_byte(o, y + (entity << 4)).add_byte(o, x)
          total += 2
          o << "\n" if total % PER_LINE == 0          
        end
      end
    end
    o << "\n"
    o
  end
  
  def simulateRLE
    @tileDataSizeRLE = 0
    currentTile = -1
    currentTileCount = 0
    for iy in 0...@height
      for ix in 0...@width      
        tile = @tileMap[iy * @width + ix]
        if tile == currentTile and currentTileCount < MAX_RLE_TILE[tile]
          #puts "added tile #{ix},#{iy}"
          currentTileCount += 1
        elsif currentTile != -1
          #puts "finished RLE block of #{currentTileCount}"
          @tileDataSizeRLE += 2 + RLE_SIZE[currentTile]
          currentTileCount = 0          
        end
        currentTile = tile
      end
    end
    
    @tileDataSizeRLE /= 8
  end

  def to_s
    o = code_header
    o << "PROGMEM const uint8_t #{@name}[] = {\n"
    o << "// width, height\n"
    o << "#{@width}, #{@height},\n"
    o << "// map type + player start y (SS00YYYY), player start x\n"
    o << meta_data
    o << "// tilemap data\n"
    o << tilemap_data
    o << "// entity count\n"
    o << "#{@entityCount},\n"
    o << "// entities data\n"
    o << entity_data
    o << "\n};\n\n"
    o
  end
end

puts "generating maps..."
files = Dir.glob("./map/**/*.json")
files.each do |fileName|
  mapData = MapData.new(fileName)
  out = mapData.to_s
  puts "#{mapData.name}: #{mapData.width}x#{mapData.height} (#{mapData.entityCount} entities) size=#{mapData.size}bytes tileData=#{mapData.tileDataSize} tileDataRLE=#{mapData.tileDataSizeRLE}"
  assets.files << out
end
puts "\n"

#
# ---------------------- img2ard ------------------------------------------------------
#

class ImageCharArray
  PER_LINE = 10
  attr :name
  attr :width
  attr :height
  attr :data
  attr :mask_data

  def initialize(img, name)
    @width = img.width
    @height = img.height
    @mask = false
    @name = name
    @data = []
    @mask_data = []
    autodetect_frames
  end

  DIMENSION_REGEX = /[-_](\d+)x(\d+)/
  def autodetect_frames
    @frames = 1
    return unless name =~ DIMENSION_REGEX

    x = $1.to_i
    y = $2.to_i

    if x != width
      fail "X size given in filename must match horizontal width of image"
    end

    if (height % y) != 0
      fail "Y size given in filename must divide evenly into total image height"
    end

    # remove the resolution from the soruce name in our header file
    name.gsub! DIMENSION_REGEX, ""

    @frames = height / y
    @height = y
  end

  def variable_name
    File.basename(name,".*").gsub("-","_")
  end

  def mask_name
    variable_name + "_mask"
  end

  def plus_mask_name
    variable_name + "_plus_mask"
  end

  def masked!
    @mask = true
  end

  def mask?
    @mask
  end

  def frame_data(data)
    return [data] if @frames == 1

    groups = []
    frame_size = data.size / @frames
    @frames.times do |i|
      groups << data.slice(i * frame_size, frame_size)
    end
    groups
  end

  def image_data(all_data)
    core = ''
    core << resolution_data
    frame_data(all_data).each_with_index do |data, fc|
      core << "// frame #{fc}\n" if @frames > 1
      data.each_with_index do |x, i|
        hex = x.to_s(16).upcase
        core << "0x" + (hex.length == 1 ? "0" : "") + hex
        core << ", " unless i == @data.size - 1
        core << "\n" if (i + 1) % PER_LINE == 0
      end
      core << "\n"
    end
    core
  end

  def resolution_data
    "// width, height\n" \
    "#{width}, #{height},\n"
  end

  def frame_count_code
    return "" unless @frames > 1
    "// #{@frames} frames\n"
  end

  def interlace(image, mask)
    a = []
    while (image.size > 0)
      a << image.shift
      a << mask.shift
    end
    a
  end

  def code_header
    o = "// #{File.basename(name)}\n"
    o << frame_count_code
    o << "// #{width}x#{height}\n"
    o
  end

  def to_s
    o = code_header
    o << "PROGMEM const uint8_t #{variable_name}[] = {\n"
    o << image_data(@data)
    o << "\n};\n\n"
    if mask?
      o << "PROGMEM const uint8_t #{mask_name}[] = {\n"
      o << image_data(@mask_data)
      o << "\n};\n\n"

      o << "PROGMEM const uint8_t #{plus_mask_name}[] = {\n"
      o << image_data(interlace(@data, @mask_data))
      o << "\n};\n\n"

    end
    o
  end
end

puts "generating bitmaps..."
files = Dir.glob("./gfx/**/*.png")
files.each do |file|
  img = ChunkyPNG::Image.from_file(file)
  # puts img.inspect
  out = ImageCharArray.new(img, file)
  puts "#{file}: #{img.width}x#{img.height}"

  bits_last_page = img.height % 8
  bytes_high = img.height / 8
  bytes_high +=1 if bits_last_page>0

  (0..bytes_high - 1).each do |ypage|
    (0..img.width - 1).each do |x|
      # how many bits does this line hold
      bits = 8
      # if we've reached the bottom there are fewer bits to load
      bits = bits_last_page if bytes_high-1==ypage and bits_last_page > 0
      byte = 0
      alpha_byte = 0
      (0..bits-1).each do |bit_height|
        px = img[x, ypage*8 + bit_height]
        # print ChunkyPNG::Color.to_hex(px)
        # right now we only care about black/white so convert to greyscale
        c = ChunkyPNG::Color.grayscale_teint(px)
        alpha = ChunkyPNG::Color.a(px)
        # puts "#{c} #{alpha}"
        # puts("#{file} #{x}, #{ypage}, #{c} #{px}")
        if c > 128 and not alpha < 128
          byte += (1 << (bit_height))
        end

        if alpha > 128 # visible
          alpha_byte += (1 << (bit_height))
          out.masked!
        end
      end
      out.mask_data << (alpha_byte)
      out.data << byte
    end
  end
  # puts out.inspect
  assets.files << out
end

#
# ---------------------- write assets ------------------------------------------------------
#

File.open("CastleBoy/" + assets.filename,"w") do |f|
  f.write assets.to_s
end
puts "\n#{assets.filename} compiled."
