#include "noise.hpp"
#include <glm/glm.hpp>
#include <math.h>
#include <random>

constexpr glm::vec2 gradients[4] = {
	glm::vec2(1.0f, 0.0f),
	glm::vec2(-1.0f, 0.0f),	
	glm::vec2(0.0f, 1.0f),
	glm::vec2(0.0f, -1.0f),
};

namespace rng {	
	void createPermutation(permutation256 &p, int seed)
	{
		int values[256];
		int count = 256;
		for(int i = 0; i < 256; i++)
			values[i] = i;

		int index = 0;
		std::minstd_rand lcg;
		lcg.seed(seed);
		while(count > 0) {
			int randindex = lcg() % count;
			p[index++] = values[randindex];
			values[randindex] = values[count - 1];
			count--;
		}
	}	
}

namespace perlin {
	glm::vec2 gradient(int x, int y, const rng::permutation256 &p)
	{
		//Taken from wikipedia
		//do some bit magic to hopefully ensure these values are not too periodic
		const unsigned w = 8 * sizeof(unsigned);
		const unsigned s = w / 2;
		unsigned a = x, b = y;
		a *= 3284157443; 
		b ^= a << s | a >> (w-s);
		b *= 1911520717; 
		a ^= b << s | b >> (w-s);
		a *= 2048419325;

		int index = p[unsigned(p[unsigned(p[a % 256] + b) % 256] % 256)];
		return gradients[index % 4];
	}

	float dotgradient(
		int gridx,
		int gridy,
		float x,
		float y,
		const rng::permutation256 &p
	) {
		glm::vec2 v = gradient(gridx, gridy, p);
		glm::vec2 d(x - float(gridx), y - float(gridy));
		return glm::dot(v, d);
	}

	float interpolate(float a, float b, float x)
	{
		return (b - a) * (3.0 - x * 2.0) * x * x + a;
	}

	float noise(float x, float y, const rng::permutation256 &p)
	{
		int
			leftx = int(floorf(x)),
			lowery = int(floorf(y)),
			rightx = leftx + 1,
			uppery = lowery + 1;
		float 
			lowerleft = dotgradient(leftx, lowery, x, y, p),
			lowerright = dotgradient(rightx, lowery, x, y, p),
			upperleft = dotgradient(leftx, uppery, x, y, p),
			upperright = dotgradient(rightx, uppery, x, y, p);
		float 
			lerpedlower = interpolate(lowerleft, lowerright, x - leftx),
			lerpedupper = interpolate(upperleft, upperright, x - leftx);
		return interpolate(lerpedlower, lerpedupper, y - lowery);
	}

	float noise(float x, float y, int repeat, const rng::permutation256 &p)
	{
		int
			leftx = int(floorf(x)) % repeat,
			lowery = int(floorf(y)) % repeat,
			rightx = (leftx + 1) % repeat,
			uppery = (lowery + 1) % repeat;
		float 
			lowerleft = dotgradient(leftx, lowery, x, y, p),
			lowerright = dotgradient(rightx, lowery, x, y, p),
			upperleft = dotgradient(leftx, uppery, x, y, p),
			upperright = dotgradient(rightx, uppery, x, y, p);
		float 
			lerpedlower = interpolate(lowerleft, lowerright, x - leftx),
			lerpedupper = interpolate(upperleft, upperright, x - leftx);
		return interpolate(lerpedlower, lerpedupper, y - lowery);
	}
}
