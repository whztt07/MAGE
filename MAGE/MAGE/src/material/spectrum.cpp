//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "material\spectrum.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Definitions
//-----------------------------------------------------------------------------
namespace mage {

	RGBSpectrum::RGBSpectrum(const XYZSpectrum &xyz) noexcept {
		x =  3.240479f * xyz.x - 1.537150f * xyz.y - 0.498535f * xyz.z;
		y = -0.969256f * xyz.x + 1.875991f * xyz.y + 0.041556f * xyz.z;
		z =  0.055648f * xyz.x - 0.204043f * xyz.y + 1.057311f * xyz.z;
	}

	XYZSpectrum::XYZSpectrum(const RGBSpectrum &rgb) noexcept {
		x = 0.412453f * rgb.x + 0.357580f * rgb.y + 0.180423f * rgb.z;
		y = 0.212671f * rgb.x + 0.715160f * rgb.y + 0.072169f * rgb.z;
		z = 0.019334f * rgb.x + 0.119193f * rgb.y + 0.950227f * rgb.z;
	}

	RGBASpectrum::RGBASpectrum(const XYZASpectrum &xyza) noexcept {

		x =  3.240479f * xyza.x - 1.537150f * xyza.y - 0.498535f * xyza.z;
		y = -0.969256f * xyza.x + 1.875991f * xyza.y + 0.041556f * xyza.z;
		z =  0.055648f * xyza.x - 0.204043f * xyza.y + 1.057311f * xyza.z;
		w = xyza.w;
	}

	XYZASpectrum::XYZASpectrum(const RGBASpectrum &rgba) noexcept {

		x = 0.412453f * rgba.x + 0.357580f * rgba.y + 0.180423f * rgba.z;
		y = 0.212671f * rgba.x + 0.715160f * rgba.y + 0.072169f * rgba.z;
		z = 0.019334f * rgba.x + 0.119193f * rgba.y + 0.950227f * rgba.z;
		w = rgba.w;
	}
}