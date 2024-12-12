/*
 * Physically Based Rendering
 * Copyright (c) 2017-2018 Micha©© Siejak
 */

#include "D3DEngine/Core/Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define NV_DDS_UTILITY_VALUES
#include "nv_dds.h"

#include <stdexcept>

#include <filesystem>
#include <fstream>
#include <vector>
namespace fs = std::filesystem;

[[nodiscard]]
static std::vector<char> read_file(const fs::path& filepath) {
  std::vector<char> result;

  try {
    // Open the file with the cursor at the end
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("File can't be opened.");
    }

    // The location of the cursor tells the size of
    // the file in bytes
    unsigned long long filesize = static_cast<unsigned long long>(file.tellg());

    if (filesize < 0) {
      throw std::runtime_error("File length is negative.");
    }

    result.resize(filesize);

    // Place the fie cursor at the beginning
    file.seekg(0);

    // Load the entire file into the buffer
    file.read(result.data(), filesize);

    // Close the file
    file.close();
  } catch (const std::exception& e) {
    printf("%s", e.what());
    return result;
  }

  // Return true
  return result;
}

Image::Image() : m_width(0), m_height(0), m_channels(0), m_hdr(false) {}

std::shared_ptr<Image> Image::fromFile(const std::string& filename,
                                       int channels) {
  std::printf("Loading image: %s\n", filename.c_str());

  std::shared_ptr<Image> image{new Image};

	if (filename.ends_with(".dds")) {
    nv_dds::Image dds;
    auto maybeError = dds.readFromFile(filename.c_str(), {});
    if (maybeError.has_value()) {
      throw std::runtime_error("Failed to load dds image file: " + filename);
		}

		image->m_width = dds.getWidth(0);
		image->m_height = dds.getHeight(0);
    image->m_levels = dds.getNumMips();
    image->m_faces = dds.getNumFaces();
    

		if (((dds.getFileInfo().ddsh.dwCaps2 & nv_dds::DDSCAPS2_CUBEMAP) != 0) &&
        (dds.getFileInfo().ddsh.dwCaps2 & nv_dds::DDSCAPS2_CUBEMAP_ALL_FACES) ==
            nv_dds::DDSCAPS2_CUBEMAP_ALL_FACES) {
      
			image->m_channels = 4;
      image->m_hdr = true;

			// Calculate the total size of the subresources
      size_t faceOffset{0};
			size_t linearSize{0};
			for (int i = 0; i < image->m_faces; ++i) {
        for (int j = 0; j < image->m_levels; ++j) {
          linearSize += dds.subresource(j, 0, i).data.size();
        }
        if (faceOffset <= 0) faceOffset = linearSize;
			}

			// Copy the pixels
			char* pixels = (char*)malloc(linearSize);
      size_t offset{0};
      for (int i = 0; i < image->m_faces; ++i) {
        for (int j = 0; j < image->m_levels; ++j) {
          memcpy(pixels + offset, dds.subresource(j, 0, i).data.data(),
                 dds.subresource(j, 0, i).data.size());
          offset += dds.subresource(j, 0, i).data.size();
        }
      }
			image->m_pixels.reset((uint8_t*)pixels);
    } else {
      image->m_channels = 4;
      image->m_hdr = false;

      // Calculate the total size of the subresources
      size_t linearSize{0};
      for (int j = 0; j < image->m_levels; ++j) {
        linearSize += dds.subresource(j, 0, 0).data.size();
      }

			// Copy the pixels
      char* pixels = (char*)malloc(linearSize);
      size_t offset{0};
      for (int j = 0; j < image->m_levels; ++j) {
        memcpy(pixels + offset, dds.subresource(j, 0, 0).data.data(),
                dds.subresource(j, 0, 0).data.size());
        offset += dds.subresource(j, 0, 0).data.size();
      }
      image->m_pixels.reset((uint8_t*)pixels);
		}

    return image;
  } else {
    if (stbi_is_hdr(filename.c_str())) {
      float* pixels =
          stbi_loadf(filename.c_str(), &image->m_width, &image->m_height,
                     &image->m_channels, channels);
      if (pixels) {
        image->m_pixels.reset(reinterpret_cast<unsigned char*>(pixels));
        image->m_hdr = true;
      }
    } else {
      unsigned char* pixels =
          stbi_load(filename.c_str(), &image->m_width, &image->m_height,
                    &image->m_channels, channels);
      if (pixels) {
        image->m_pixels.reset(pixels);
        image->m_hdr = false;
      }
    }
    if (channels > 0) {
      image->m_channels = channels;
    }

    if (!image->m_pixels) {
      throw std::runtime_error("Failed to load image file: " + filename);
    }
    return image;
	}
}
