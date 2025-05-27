/*
 *  pent_shp.cpp - Aseprite plugin converter for SHP files
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
 */
#define HAVE_CRUSADER
// Pentagram includes for Ultima 8 support
#include "pent_include.h"
#include "util.h"
#include "FileSystem.h"
#include "ConvertShape.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "XFormBlend.h"
#include "Palette.h"
#include "u8/ConvertShapeU8.h"
#include "crusader/ConvertShapeCrusader.h"

#include <fcntl.h>
#include <png.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#endif

#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <vector>

using Palette_t = std::array<unsigned char, 768>;

namespace {
    // Forward declarations
    void generateU8Palette(unsigned char* palette);
    void generateCrusaderPalette(unsigned char* palette);

    // Add missing typedef for GIMP compatibility
    typedef unsigned char guchar;

    // Get the directory where the converter executable is located
    std::string getExecutableDirectory() {
        char path[1024];
        ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (len != -1) {
            path[len] = '\0';
            std::string execPath(path);
            size_t lastSlash = execPath.find_last_of('/');
            if (lastSlash != std::string::npos) {
                return execPath.substr(0, lastSlash + 1);
            }
        }
        
        // Fallback for macOS - use _NSGetExecutablePath
        #ifdef __APPLE__
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            std::string execPath(path);
            // Resolve symlinks
            char realPath[PATH_MAX];
            if (realpath(path, realPath)) {
                execPath = std::string(realPath);
            }
            size_t lastSlash = execPath.find_last_of('/');
            if (lastSlash != std::string::npos) {
                return execPath.substr(0, lastSlash + 1);
            }
        }
        #endif
        
        // Fallback to current directory
        return "./";
    }

    // Load palette from file with better diagnostics
    bool loadPaletteFromFile(Pentagram::Palette* pal, const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            return false;
        }
        
        // Check file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (fileSize < 772) {  // 4 bytes header + 768 bytes palette
            std::cerr << "Palette file too small: " << fileSize << " bytes" << std::endl;
            return false;
        }
        
        // Read and display the header
        uint32_t header;
        file.read(reinterpret_cast<char*>(&header), 4);
        
        // Read 768 bytes of palette data directly into pal->palette
        file.read(reinterpret_cast<char*>(pal->palette), 768);
        
        if (!file) {
            std::cerr << "Failed to read palette data from file" << std::endl;
            return false;
        }
        
        file.close();
        std::cout << "Successfully loaded palette from: " << filename << std::endl;
        
        // Check for non-grayscale colors further in the palette
        bool hasColor = false;
        for (int i = 16; i < 256 && !hasColor; ++i) {
            unsigned char r = pal->palette[i*3];
            unsigned char g = pal->palette[i*3+1]; 
            unsigned char b = pal->palette[i*3+2];
            if (r != g || g != b) {
                hasColor = true;
            }
        }
        
        if (!hasColor) {
            std::cout << "WARNING: Palette appears to be entirely grayscale!" << std::endl;
        }
        
        return true;
    }

    // Alternative: Try loading without skipping the header
    bool loadPaletteFromFileNoHeader(Pentagram::Palette* pal, const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            return false;
        }
        
        // Check file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (fileSize < 768) {
            std::cerr << "Palette file too small (no header): " << fileSize << " bytes" << std::endl;
            return false;
        }
        
        // Read 768 bytes directly without skipping header
        file.read(reinterpret_cast<char*>(pal->palette), 768);
        
        if (!file) {
            std::cerr << "Failed to read palette data (no header)" << std::endl;
            return false;
        }
        
        file.close();
        std::cout << "Successfully loaded palette (no header) from: " << filename << std::endl;
        
        // Debug: Print first few colors
        std::cout << "No-header palette - first 4 colors:" << std::endl;
        for (int i = 0; i < 4; ++i) {
            std::cout << "Color " << i << ": RGB(" 
                     << (int)pal->palette[i*3] << "," 
                     << (int)pal->palette[i*3+1] << "," 
                     << (int)pal->palette[i*3+2] << ")" << std::endl;
        }
        
        return true;
    }

    // Load palette - try both header and no-header formats
    bool loadPalette(Pentagram::Palette* pal, const ConvertShapeFormat* format_ptr) { // Renamed 'format' to 'format_ptr' to avoid conflict
        // Initialize native palette like pentpal.cpp does
        for (int i = 0; i < 256; ++i) {
            guchar palHack[2];
            palHack[0] = i;
            palHack[1] = 255;
            pal->native[i] = *((uint16_t*)palHack); // Use uint16_t for clarity
        }

        std::string paletteName;
        std::string paletteFileToLoad; 

        if (format_ptr == &PentagramShapeFormat) {
            paletteName = "Pentagram";
            generateU8Palette(pal->palette); 
            std::cout << "Using generated grayscale for Pentagram format." << std::endl;
            return true; 
        }
        else if (format_ptr == &U8SKFShapeFormat || 
                 format_ptr == &U8ShapeFormat || 
                 format_ptr == &U82DShapeFormat) {
            paletteName = "Ultima8";
            paletteFileToLoad = "u8pal.pal";
        }
        else if (format_ptr == &CrusaderShapeFormat || 
                 format_ptr == &Crusader2DShapeFormat) {
            paletteName = "Crusader";
            paletteFileToLoad = "crusader.pal"; 
        }
        else {
            std::cerr << "Unknown format, attempting fallback with u8pal.pal" << std::endl;
            paletteName = "Unknown (U8 fallback)";
            paletteFileToLoad = "u8pal.pal";
        }

        std::cout << "Attempting to load base palette for format: " << paletteName << ", file: " << paletteFileToLoad << std::endl;

        std::string execDir = getExecutableDirectory();
        std::vector<std::string> searchPaths = {
            execDir + paletteFileToLoad, // Search in executable's directory
            paletteFileToLoad            // Search in current working directory
        };
         bool loadedSuccessfully = false;
        for (const auto& path : searchPaths) {
            std::cout << "Trying base palette: " << path << std::endl;
            if (loadPaletteFromFile(pal, path)) { 
                loadedSuccessfully = true;
                break;
            }
        }

        if (!loadedSuccessfully) {
            std::cerr << "Warning: Could not find " << paletteFileToLoad << " for " << paletteName 
                     << ", using generated fallback." << std::endl;
            
            if (paletteName.find("Crusader") != std::string::npos) {
                generateCrusaderPalette(pal->palette);
            } else { 
                generateU8Palette(pal->palette);
            }
        }
        return true; 
    }

    // Generate a reasonable Ultima 8-style palette as fallback
    void generateU8Palette(unsigned char* palette) {
        // Create a basic VGA-style palette for U8
        // Color 0: Black
        palette[0] = palette[1] = palette[2] = 0;
        
        // Colors 1-15: Basic VGA colors
        unsigned char vga_colors[15][3] = {
            {170, 0, 0},     // Dark Red
            {0, 170, 0},     // Dark Green  
            {170, 85, 0},    // Brown
            {0, 0, 170},     // Dark Blue
            {170, 0, 170},   // Dark Magenta
            {0, 170, 170},   // Dark Cyan
            {170, 170, 170}, // Light Gray
            {85, 85, 85},    // Dark Gray
            {255, 85, 85},   // Light Red
            {85, 255, 85},   // Light Green
            {255, 255, 85},  // Yellow
            {85, 85, 255},   // Light Blue
            {255, 85, 255},  // Light Magenta
            {85, 255, 255},  // Light Cyan
            {255, 255, 255}  // White
        };
        
        for (int i = 0; i < 15; ++i) {
            palette[(i + 1) * 3] = vga_colors[i][0];
            palette[(i + 1) * 3 + 1] = vga_colors[i][1];
            palette[(i + 1) * 3 + 2] = vga_colors[i][2];
        }
        
        // Fill rest with gradients typical of U8
        for (int i = 16; i < 256; ++i) {
            if (i < 64) {
                // Skin tones
                int level = (i - 16) * 4;
                palette[i * 3] = std::min(255, 200 + level);
                palette[i * 3 + 1] = std::min(255, 150 + level);
                palette[i * 3 + 2] = std::min(255, 100 + level);
            } else if (i < 128) {
                // Earth tones  
                int level = (i - 64) * 4;
                palette[i * 3] = std::min(255, 100 + level);
                palette[i * 3 + 1] = std::min(255, 80 + level);
                palette[i * 3 + 2] = std::min(255, 40 + level);
            } else {
                // Grayscale ramp
                int gray = (i - 128) * 2;
                palette[i * 3] = palette[i * 3 + 1] = palette[i * 3 + 2] = std::min(255, gray);
            }
        }
    }

    // Generate Crusader palette (simpler than U8)
    void generateCrusaderPalette(unsigned char* palette) {
        // Crusader uses more metallic/industrial colors
        for (int i = 0; i < 256; ++i) {
            if (i == 0) {
                palette[i * 3] = palette[i * 3 + 1] = palette[i * 3 + 2] = 0;
            } else {
                // More metallic/gray tones for Crusader
                int level = i;
                palette[i * 3] = std::min(255, level);
                palette[i * 3 + 1] = std::min(255, (level * 3) / 4);
                palette[i * 3 + 2] = std::min(255, level / 2);
            }
        }
    }

    // Sanitize a filename to prevent path traversal attacks
    std::string sanitizeFilename(const std::string& input) {
        std::string sanitized;
        sanitized.reserve(input.size());

        for (const char c : input) {
            // Allow only alphanumeric characters, underscore, hyphen, and period
            auto uchr = static_cast<unsigned char>(c);
            if (isalnum(uchr) != 0 || c == '_' || c == '-' || c == '.') {
                sanitized += c;
            } else {
                // Replace potentially dangerous characters with underscore
                sanitized += '_';
            }
        }

        return sanitized;
    }

    // Sanitize a file path to prevent directory traversal attacks
    std::string sanitizeFilePath(const std::string& path) {
        if (path.empty()) {
            return "";
        }

        std::string sanitized;
        std::string filename;

        // Extract the path and filename
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            sanitized = path.substr(0, lastSlash + 1);
            filename  = path.substr(lastSlash + 1);
        } else {
            filename = path;
        }

        // Sanitize just the filename part
        sanitized += sanitizeFilename(filename);

        return sanitized;
    }

    // Save a frame to a PNG file
    bool saveFrameToPNG(
            const std::string& filename, const unsigned char* data,
            size_t width, size_t height, const Palette_t& palette) {
        // Create file for writing with restricted permissions
        FILE* fp = nullptr;
#ifdef _WIN32
        // Windows implementation
        fp = fopen(filename.c_str(), "wb");
#else
        // Unix/Mac implementation with restricted permissions (0644 = rw-r--r--)
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        fp     = (fd >= 0) ? fdopen(fd, "wb") : nullptr;
        if ((fp == nullptr) && fd >= 0) {
            close(fd);
        }
#endif

        if (fp == nullptr) {
            std::cerr << "Error: Failed to open file for writing: " << filename
                      << std::endl;
            return false;
        }

        // Initialize libpng structures
        png_structp png_ptr = png_create_write_struct(
                PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png_ptr == nullptr) {
            fclose(fp);
            return false;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == nullptr) {
            png_destroy_write_struct(&png_ptr, nullptr);
            fclose(fp);
            return false;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            return false;
        }

        png_init_io(png_ptr, fp);

        // Set image attributes for indexed color PNG
        png_set_IHDR(
                png_ptr, info_ptr, width, height,
                8,    // bit depth
                PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        // Set up the palette
        png_color png_palette[256];
        for (size_t i = 0; i < 256; i++) {  // Changed from std::size
            png_palette[i].red   = palette[(i * 3)];
            png_palette[i].green = palette[(i * 3) + 1];
            png_palette[i].blue  = palette[(i * 3) + 2];
        }

        png_set_PLTE(png_ptr, info_ptr, png_palette, 256);  // Changed from std::size

        // Create full transparency array - 0 is fully transparent, 255 is fully opaque
        png_byte trans[256];
        for (size_t i = 0; i < 256; i++) {  // Changed from std::size
            // Make only index 255 transparent
            trans[i] = (i == 255) ? 0 : 255;
        }

        // Set transparency for all palette entries, with index 255 being transparent
        png_set_tRNS(png_ptr, info_ptr, trans, 256, nullptr);  // Changed from std::size

        // Write the PNG info
        png_write_info(png_ptr, info_ptr);

        // Allocate memory for row pointers
        std::vector<png_bytep> row_pointers(height);
        for (size_t y = 0; y < height; ++y) {
            row_pointers[y] = const_cast<png_bytep>(&data[y * width]);
        }

        // Write image data
        png_write_image(png_ptr, row_pointers.data());
        png_write_end(png_ptr, nullptr);

        // Clean up
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);

        std::cout << "Successfully saved PNG: " << filename << std::endl;
        return true;
    }

    // Paint frame function - with refined RLE decoding to match SoftRenderSurface.inl xpos advancement
    void paintFrame(Shape* s, uint32 framenum, uint8* pixels,
                   uint32 pitch, sint32 x_offset_on_canvas, sint32 y_offset_on_canvas, 
                   uint32 canvas_buf_width, uint32 canvas_buf_height) {
        
        ShapeFrame* frame = s->getFrame(framenum);
        if (!frame) {
            std::cout << "Warning: Frame " << framenum << " is null" << std::endl;
            return;
        }
        
        if (!frame->line_offsets || !frame->rle_data) {
            std::cout << "Warning: Frame " << framenum << " has no RLE data or line_offsets" << std::endl;
            return;
        }
        
        memset(pixels, 255, canvas_buf_width * canvas_buf_height);  // Fill with transparent (index 255)

        // Variables for SoftRenderSurface.inl - matching names from its top section
        // 's' and 'framenum' are parameters.
        // 'pixels' is our target buffer.
        // 'pitch' is the canvas pitch.
        // 'x' and 'y' are the top-left coordinates for drawing the frame on the canvas.
        sint32 x = x_offset_on_canvas;
        sint32 y = y_offset_on_canvas;

        // clip_window setup (if not using NO_CLIPPING)
        // For simplicity here, assuming NO_CLIPPING. If you need clipping,
        // clip_window must be properly defined.
        // struct { int x, y, w, h; } clip_window = {0, 0, (int)canvas_buf_width, (int)canvas_buf_height};


        // --- Preprocessor defines for pent_shp_rendersurface.inl ---
        #define PENT_SHP_OUTPUT_8BIT_INDICES // Our new flag to write indices
        #define uintX uint8_t                // Outputting 8-bit indices
        #define NO_CLIPPING                  // Simplifies setup; remove if you implement clipping
        // #undef XFORM_SHAPES               // Ensure XFORM_SHAPES is not defined, as gamePal is already transformed
                                             // and PENT_SHP_OUTPUT_8BIT_INDICES bypasses xform_pal anyway.
        // #undef FLIP_SHAPES                // Define or undefine as needed

        // This variable is expected by the .inl file to choose between palette types.
        // Since gamePal is already transformed and PENT_SHP_OUTPUT_8BIT_INDICES ignores
        // the pal/xform_pal lookups, its value here is less critical but should be defined.
        bool untformed_pal = false; 
        
        // Include the modified rendering logic
        #include "pent_shp_rendersurface.inl" // Include your local, modified copy

        // --- Undefine to prevent conflicts elsewhere ---
        #undef PENT_SHP_OUTPUT_8BIT_INDICES
        #undef uintX
        #undef NO_CLIPPING
    }

    // Import function
    bool importU8SHP(const std::string& shpFilename, const std::string& outputPath) {
        try {
            FileSystem* filesys = FileSystem::get_instance();
            if (!filesys) {
                filesys = new FileSystem(true); 
            }

            IDataSource* ids = filesys->ReadFile(shpFilename);
            if (!ids) {
                std::cerr << "Error: Could not open input file: " << shpFilename << std::endl;
                return false;
            }

            ids->seek(0); 
            uint32_t read_size = ids->getSize(); 
            const ConvertShapeFormat* read_format = nullptr;
            
            std::string fn_lower = shpFilename;
            std::transform(fn_lower.begin(), fn_lower.end(), fn_lower.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            
            if (fn_lower.length() >= 4 && fn_lower.substr(fn_lower.length() - 4) == ".skf") {
                std::cout << "File extension is .skf, assuming U8SKFShapeFormat." << std::endl;
                read_format = &U8SKFShapeFormat; 
            }

            if (!read_format) {
                std::cout << "Extension not .skf or initial check failed, using Shape::DetectShapeFormat." << std::endl;
                ids->seek(0); 
                read_format = Shape::DetectShapeFormat(ids, read_size);
            }
            ids->seek(0); 

            std::cout << "Detected format for file: " << shpFilename << std::endl;
            if (read_format == &U8SKFShapeFormat) {
                std::cerr << "SKF format is not supported!" << std::endl;
                delete ids;
                return false;
            }
            // Print format for non-SKF files
            else if (read_format == &U8ShapeFormat) std::cout << "  Format: U8ShapeFormat" << std::endl;
            else if (read_format == &U82DShapeFormat) std::cout << "  Format: U82DShapeFormat" << std::endl;
            else if (read_format == &CrusaderShapeFormat) std::cout << "  Format: CrusaderShapeFormat" << std::endl;
            else if (read_format == &Crusader2DShapeFormat) std::cout << "  Format: Crusader2DShapeFormat" << std::endl;
            else if (read_format == &PentagramShapeFormat) std::cout << "  Format: PentagramShapeFormat" << std::endl;
            else {
                std::cout << "  Format: Unknown or unsupported (pointer mismatch)" << std::endl;
                delete ids; // Clean up if format is truly unknown/unsupported by non-SKF path
                return false;
            }


            std::string baseFilenameForOutput = shpFilename;
            size_t lastSlash = baseFilenameForOutput.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                baseFilenameForOutput = baseFilenameForOutput.substr(lastSlash + 1);
            }
            size_t lastDot = baseFilenameForOutput.find_last_of('.');
            if (lastDot != std::string::npos) {
                baseFilenameForOutput = baseFilenameForOutput.substr(0, lastDot);
            }
            baseFilenameForOutput = sanitizeFilename(baseFilenameForOutput);

            std::string outputDir;
            size_t lastPathSep = outputPath.find_last_of("/\\");
            if (lastPathSep != std::string::npos) {
                outputDir = outputPath.substr(0, lastPathSep + 1);
            }
            std::string outputBaseName = outputPath.substr(lastPathSep + 1);


            // Non-SKF U8SHP, Crusader, Pentagram SHP handling
            Shape shape(ids, read_format); // ids ownership transferred to Shape
            Pentagram::Palette* gamePal = new Pentagram::Palette;

            if (read_format == &U8ShapeFormat || read_format == &U82DShapeFormat) {
                std::cout << "Palette path: Using raw u8pal.pal (scaled to 8-bit) for U8/U82D format..." << std::endl;
                Pentagram::Palette tempRawPal;
                if (!loadPalette(&tempRawPal, read_format)) { /* error */ delete gamePal; return false; }
                for (int i = 0; i < 256; ++i) {
                    gamePal->palette[i*3+0] = (tempRawPal.palette[i*3+0] * 255 + 31) / 63;
                    gamePal->palette[i*3+1] = (tempRawPal.palette[i*3+1] * 255 + 31) / 63;
                    gamePal->palette[i*3+2] = (tempRawPal.palette[i*3+2] * 255 + 31) / 63;
                }
            } else if (read_format == &CrusaderShapeFormat || read_format == &Crusader2DShapeFormat) {
                std::cout << "Palette path: Applying U8XFormPal to crusader.pal for Crusader format..." << std::endl;
                Pentagram::Palette tempRawPal;
                if (!loadPalette(&tempRawPal, read_format)) { /* error */ delete gamePal; return false; }
                IBufferDataSource dataSource(tempRawPal.palette, 768);
                IBufferDataSource xformSource(U8XFormPal, sizeof(U8XFormPal));
                try { gamePal->load(dataSource, xformSource); }
                catch (const std::exception& e) { /* error */ delete gamePal; return false; }
            } else if (read_format == &PentagramShapeFormat) {
                std::cout << "Palette path: Pentagram format logic..." << std::endl;
                const Pentagram::Palette* shapeInternalPalette = shape.getPalette();
                if (shapeInternalPalette) {
                    memcpy(gamePal->palette, shapeInternalPalette->palette, sizeof(gamePal->palette));
                } else {
                    std::cout << "  Pentagram sub-path: Falling back to raw u8pal.pal (scaled to 8-bit)." << std::endl;
                    Pentagram::Palette tempRawU8Pal;
                    if (!loadPalette(&tempRawU8Pal, &U8ShapeFormat)) { /* error */ delete gamePal; return false; }
                    for (int i = 0; i < 256; ++i) {
                        gamePal->palette[i*3+0] = (tempRawU8Pal.palette[i*3+0] * 255 + 31) / 63;
                        gamePal->palette[i*3+1] = (tempRawU8Pal.palette[i*3+1] * 255 + 31) / 63;
                        gamePal->palette[i*3+2] = (tempRawU8Pal.palette[i*3+2] * 255 + 31) / 63;
                    }
                }
            } else {
                // This case should ideally not be reached if the format check above is exhaustive for non-SKF
                std::cout << "Palette path: Unknown non-SKF format. Using generated U8 grayscale palette." << std::endl;
                generateU8Palette(gamePal->palette);
            }
            
            shape.setPalette(gamePal);

            sint32 min_x = 0, min_y = 0;
            sint32 max_width = 0, max_height = 0;
            bool first_frame = true;
            for (uint32_t i = 0; i < shape.frameCount(); ++i) {
                ShapeFrame* current_frame = shape.getFrame(i);
                if (current_frame) {
                    if (first_frame) {
                        min_x = current_frame->xoff; min_y = current_frame->yoff;
                        max_width = current_frame->width + current_frame->xoff;
                        max_height = current_frame->height + current_frame->yoff;
                        first_frame = false;
                    } else {
                        if (current_frame->xoff < min_x) min_x = current_frame->xoff;
                        if (current_frame->yoff < min_y) min_y = current_frame->yoff;
                        if (current_frame->width + current_frame->xoff > max_width)
                            max_width = current_frame->width + current_frame->xoff;
                        if (current_frame->height + current_frame->yoff > max_height)
                            max_height = current_frame->height + current_frame->yoff;
                    }
                }
            }
            if (first_frame && shape.frameCount() > 0) { /* error, no valid frames with dimensions */ delete gamePal; return false; }
            if (shape.frameCount() == 0) { /* no frames to process */ delete gamePal; return true; }


            sint32 canvas_width = max_width - min_x;
            sint32 canvas_height = max_height - min_y;
            if(canvas_width <=0 || canvas_height <=0) { /* error */ delete gamePal; return false; }

            std::cout << "Canvas dimensions: " << canvas_width << "x" << canvas_height << std::endl;

            Palette_t palette_for_png;
            for (int i = 0; i < 256; ++i) {
                palette_for_png[i * 3    ] = gamePal->palette[i * 3    ];
                palette_for_png[i * 3 + 1] = gamePal->palette[i * 3 + 1];
                palette_for_png[i * 3 + 2] = gamePal->palette[i * 3 + 2];
            }

            uint32_t output_file_index = 0; // Counter for actual output files
            for (uint32_t i = 0; i < shape.frameCount(); ++i) {
                ShapeFrame* current_frame_to_export = shape.getFrame(i);
                if (!current_frame_to_export) continue;
                
                std::vector<uint8_t> imageData(canvas_width * canvas_height, 255); // Initialized to transparent
                uint32_t pitch = canvas_width;
                paintFrame(&shape, i, imageData.data(), pitch, 
                          current_frame_to_export->xoff - min_x, 
                          current_frame_to_export->yoff - min_y, 
                          canvas_width, canvas_height);

                // Check if the frame is entirely transparent (all pixels are index 255)
                bool frameIsEmpty = true;
                for (const auto& pixel_value : imageData) {
                    if (pixel_value != 255) { // 255 is the transparency index used by memset in paintFrame
                        frameIsEmpty = false;
                        break;
                    }
                }

                // Construct filenames using the output_file_index for numbering if the frame is not empty
                // The log message for skipped frames will still use 'i' to refer to the original frame index.
                std::string frameFilenameForLog = outputDir + outputBaseName + "_" + baseFilenameForOutput + "_" + std::to_string(i) + ".png";

                if (frameIsEmpty) {
                    std::cout << "Frame " << i << " (" << frameFilenameForLog << ") is empty, skipping file creation." << std::endl;
                } else {
                    std::string frameFilename = outputDir + outputBaseName + "_" + baseFilenameForOutput + "_" + std::to_string(output_file_index) + ".png";
                    std::string metadataFilename = outputDir + outputBaseName + "_" + baseFilenameForOutput + "_" + std::to_string(output_file_index) + ".meta";

                    if (!saveFrameToPNG(frameFilename, imageData.data(), canvas_width, canvas_height, palette_for_png)) {
                        std::cerr << "Error: Failed to save frame " << i << " (output index " << output_file_index << ") to PNG" << std::endl;
                    } else {
                        // Only create metafile if PNG was saved
                        FILE* metaFile = fopen(metadataFilename.c_str(), "w");
                        if (metaFile) {
                            fprintf(metaFile, "frame_width=%d\nframe_height=%d\n", current_frame_to_export->width, current_frame_to_export->height);
                            // fprintf(metaFile, "offset_x=%d\noffset_y=%d\n", current_frame_to_export->xoff - min_x, current_frame_to_export->yoff - min_y); // Removed
                            fprintf(metaFile, "format=%s\n", read_format->name); 
                            fprintf(metaFile, "canvas_width=%d\ncanvas_height=%d\n", canvas_width, canvas_height);
                            fprintf(metaFile, "original_frame_index=%u\n", i); // Add original index to metadata
                            fclose(metaFile);
                        } else {
                            std::cerr << "Error: Failed to create metadata file for frame " << i << " (output index " << output_file_index << "): " << metadataFilename << std::endl;
                        }
                    }
                    output_file_index++; // Increment for the next saved file
                }
            }
            delete gamePal; 
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error importing shape: " << e.what() << std::endl;
            return false;
        }
    }

    // Export is not implemented for Ultima 8 shapes
    bool exportU8SHP(
            const std::string& basePath, 
            const std::string& outputShpFilename,
            const std::string& metadataFile) {
        std::cerr << "Error: Export to Ultima 8 SHP format is not currently supported" << std::endl;
        return false;
    }

}    // namespace

// Main function
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  Import mode: " << argv[0]
                  << " import <u8_shp_file> <output_base_path>"
                  << std::endl;
        std::cout << "Note: Only Ultima 8 shape formats are supported" << std::endl;
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "import") {
        if (argc < 4) {
            std::cerr << "Import mode requires Ultima 8 SHP file and output base path" << std::endl;
            return 1;
        }

        std::string shpFilename = argv[2];
        std::string outputBasePath = argv[3];  // This should be the base path without extension

        std::cout << "Loading Ultima 8 SHP file: " << shpFilename << std::endl;
        std::cout << "Output base path: " << outputBasePath << std::endl;

        if (!importU8SHP(shpFilename, outputBasePath)) {
            std::cerr << "Failed to import Ultima 8 SHP" << std::endl;
            return 1;
        }

        return 0;
    }
    else {
        std::cerr << "Unknown mode. Use 'import'." << std::endl;
        return 1;
    }
}
