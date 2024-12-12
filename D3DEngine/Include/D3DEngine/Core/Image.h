/*
 * Physically Based Rendering
 * Copyright (c) 2017-2018 Micha�� Siejak
 */

#pragma once

#include <cassert>
#include <memory>
#include <string>

class Image
{
public:
	static std::shared_ptr<Image> fromFile(const std::string& filename, int channels = 4);

	int width() const { return m_width; }
	int height() const { return m_height; }
	int channels() const { return m_channels; }
  int levels() const { return m_levels; }
  int faces() const { return m_faces; }
  int faceOffset() const { return m_faceOffset; }
	int bytesPerPixel() const { return m_channels * (m_hdr ? sizeof(float) : sizeof(unsigned char)); }
	int pitch() const { return m_width * bytesPerPixel(); }

	bool isHDR() const { return m_hdr; }

	template<typename T>
	const T* pixels() const
	{
		return reinterpret_cast<const T*>(m_pixels.get());
	}

private:
	Image();

	int m_width;
	int m_height;
	int m_channels;
	int m_levels;
  int m_faces;
  int m_faceOffset;
	bool m_hdr;
	std::unique_ptr<unsigned char> m_pixels;
};
