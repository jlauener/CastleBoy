#!/usr/bin/env ruby
require 'rubygems'
require 'chunky_png'

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
    o << "PROGMEM const unsigned char #{variable_name}[] = {\n"
    o << image_data(@data)
    o << "\n};\n\n"
    if mask?
      o << "PROGMEM const unsigned char #{mask_name}[] = {\n"
      o << image_data(@mask_data)
      o << "\n};\n\n"

      o << "PROGMEM const unsigned char #{plus_mask_name}[] = {\n"
      o << image_data(interlace(@data, @mask_data))
      o << "\n};\n\n"

    end
    o
  end
end

resource = AssetFile.new("assets")

files = Dir.glob("./assets/**/*.png")
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
  resource.files << out
end

File.open(resource.filename,"w") do |f|
  f.write resource.to_s
end
puts "\n#{resource.filename} compiled."
