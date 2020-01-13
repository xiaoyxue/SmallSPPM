#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "utils.h"
#include "linagl.h"
#include <vector>
#include "svpng.inc"
#include "scalar.h"
#include "array.h"
#include <fstream>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

NAMESPACE_BEGIN

class ImageIO {
public:
	struct BmpHeader
	{
		uint32   mFileSize;        // Size of file in bytes
		uint32   mReserved01;      // 2x 2 reserved bytes
		uint32   mDataOffset;      // Offset in bytes where data can be found (54)

		uint32   mHeaderSize;      // 40B
		uint32   mWidth;           // Width in pixels
		uint32   mHeight;          // Height in pixels

		int16    mColorPlates;     // Must be 1
		int16    mBitsPerPixel;    // We use 24bpp
		uint32   mCompression;     // We use BI_RGB ~ 0, uncompressed
		uint32   mImageSize;       // mWidth x mHeight x 3B
		uint32   mHorizRes;        // Pixels per meter (75dpi ~ 2953ppm)
		uint32   mVertRes;         // Pixels per meter (75dpi ~ 2953ppm)
		uint32   mPaletteColors;   // Not using palette - 0
		uint32   mImportantColors; // 0 - all are important
	};

	static void WritePngFile(const std::string &filename, const std::vector<Vec3> &image, uint32 resX, uint32 resY, real gamma = 2.2) {
		FILE* f = fopen(filename.c_str(), "wb");
		typedef unsigned char byte;
		byte *pngImage = new byte[resX * resY * 3];
		for (size_t j = 0; j < resY; ++j) {
			for (size_t i = 0; i < resX; ++i) {
				size_t index = 3 * j * resX + 3 * i;
				size_t imageBufferIndex = j * resX + i;
				pngImage[index] = (byte)(toInt(image[imageBufferIndex].x, gamma));
				pngImage[index + 1] = (byte)(toInt(image[imageBufferIndex].y, gamma));
				pngImage[index + 2] = (byte)(toInt(image[imageBufferIndex].z, gamma));
			}
		}
		svpng(f, resX, resY, pngImage, 0);
		delete[] pngImage;
		fclose(f);
	}

	static void WriteBmpFile(const std::string &filename, const std::vector<Vec3> &image, uint32 resX, uint32 resY, real gamma = 2.2)
	{
		std::ofstream bmp(filename.c_str(), std::ios::binary);
		BmpHeader header;
		bmp.write("BM", 2);
		header.mFileSize = (uint32)(sizeof(BmpHeader) + 2) + resX * resY * 3;
		header.mReserved01 = 0;
		header.mDataOffset = (uint32)(sizeof(BmpHeader) + 2);
		header.mHeaderSize = 40;
		header.mWidth = resX;
		header.mHeight = resY;
		header.mColorPlates = 1;
		header.mBitsPerPixel = 24;
		header.mCompression = 0;
		header.mImageSize = resX * resY * 3;
		header.mHorizRes = 2953;
		header.mVertRes = 2953;
		header.mPaletteColors = 0;
		header.mImportantColors = 0;
		bmp.write((char*)&header, sizeof(header));

		for (size_t y = 0; y < resY; y++)
		{
			for (size_t x = 0; x < resX; x++)
			{
				const Vec3 &rgbF = image[x + (resY - y - 1) * resX];
				typedef unsigned char byte;
				byte bgrB[3];
				bgrB[0] = byte(toInt(rgbF.z, gamma));
				bgrB[1] = byte(toInt(rgbF.y, gamma));
				bgrB[2] = byte(toInt(rgbF.x, gamma));

				bmp.write((char*)&bgrB, sizeof(bgrB));
			}
		}
	}

	static int toInt(real x, real gamma = 2.2f) {
		return int(pow(Clamp(x, 0.f, 1.f), 1 / gamma) * 255 + .5f);
	}

	static std::vector<Vec3> LoadImage(const std::string& filename, int resX, int resY) {
		int channels;
		float* data = stbi_loadf(filename.c_str(), &resX, &resY, &channels, 0);
		std::vector<Vec3> image(resX * resY);
		for (int y = 0; y < resY; ++y) {
			for (int x = 0; x < resX; ++x) {
				int idx = y * resX + x;
				for (int k = 0; k < channels; ++k) {
					image[idx][k] = data[idx * channels + k];
				}
			}
		}
		return image;
	}

	static Array2D<Vec3> LoadImage(const std::string& filename) {
		int resX, resY, channels;
		float* data = stbi_loadf(filename.c_str(), &resX, &resY, &channels, 0);
		std::vector<Vec3> texture(resX * resY);
		for (int y = 0; y < resY; ++y) {
			for (int x = 0; x < resX; ++x) {
				int idx = y * resX + x;
				for (int k = 0; k < channels; ++k) {
					texture[idx][k] = data[idx * channels + k];
				}
			}
		}
		return Array2D<Vec3>(texture, resY, resX);
	}

	static Array2D<Vec3> LoadTexture(std::string filename) {
		int resX, resY, channels;
		float* data = stbi_loadf(filename.c_str(), &resX, &resY, &channels, 0);
		std::vector<Vec3> texture(resX * resY);
		for (int y = 0; y < resY; ++y) {
			for (int x = 0; x < resX; ++x) {
				int idx_texture = y * resX + x;
				int idx = (resY - 1 - y) * resX + x;
				for (int k = 0; k < channels; ++k) {
					texture[idx_texture][k] = data[idx * channels + k];
				}
			}
		}
		return Array2D<Vec3>(texture, resY, resX);
	}

	static void WriteImage(const std::string& filename, const std::vector<Vec3>& image, uint32 resX, uint32 resY, real gamma = 2.2) {
		static const int channels = 3;
		std::string suffix = filename.substr(filename.length() - 4);
		if (suffix == ".hdr") {
			std::vector<float> dataToWrite(resX * resY * channels);
			for (int y = 0; y < resY; ++y) {
				for (int x = 0; x < resX; ++x) {
					for (int k = 0; k < channels; ++k) {
						int idx = y * resX + x;
						dataToWrite[idx * channels + k] = image[idx][k];
					}
				}
			}
			stbi_write_hdr(filename.c_str(), resX, resY, channels, &dataToWrite[0]);
		}
		else {
			std::vector<unsigned char> dataToWrite(resX * resY * channels);
			for (int y = 0; y < resY; ++y) {
				for (int x = 0; x < resX; ++x) {
					for (int k = 0; k < channels; ++k) {
						int idx = y * resX + x;
						dataToWrite[idx * channels + k] = unsigned char(toInt(image[idx][k]));
					}
				}
			}
			if (suffix == ".bmp") {
				stbi_write_bmp(filename.c_str(), resX, resY, channels, &dataToWrite[0]);
			}
			else if (suffix == ".png") {
				stbi_write_png(filename.c_str(), resX, resY, channels, &dataToWrite[0], channels * resX);
			}
		}
	}

};

NAMESPACE_END