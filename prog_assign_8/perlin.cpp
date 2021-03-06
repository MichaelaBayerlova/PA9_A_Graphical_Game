#include "perlin.h"

/*
	@brief:		Constructor with default values for the argument
*/
Perlin::Perlin(unsigned int oct = 1, float freq = 1.0f, float per = 0.45f)
{
	octaves = oct;
	frequency = freq;
	persistence = per;
	doublePermutation();
}

/*
	@brief:		Destructor 
*/
Perlin::~Perlin(){}


double Perlin::generateNoise(double x, double y, double z) {

	int xi = (int)x & 255;								// Calculate the "unit cube" that the point asked will be located in
	int yi = (int)y & 255;								// The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
	int zi = (int)z & 255;								// plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
	
	double xf = x - (int)x;								// We also fade the location to smooth the result.
	double yf = y - (int)y;
	double zf = z - (int)z;

	double u = fade(xf);
	double v = fade(yf);
	double w = fade(zf);

	int aaa, aba, aab, abb, baa, bba, bab, bbb;

	aaa = doubledPermutation[doubledPermutation[doubledPermutation[xi] + yi] + zi];
	aba = doubledPermutation[doubledPermutation[doubledPermutation[xi] + yi+1] + zi];
	aab = doubledPermutation[doubledPermutation[doubledPermutation[xi] + yi] + zi+1];
	abb = doubledPermutation[doubledPermutation[doubledPermutation[xi] + yi+1] + zi+1];
	baa = doubledPermutation[doubledPermutation[doubledPermutation[xi+1] + yi] + zi];
	bba = doubledPermutation[doubledPermutation[doubledPermutation[xi+1] + yi+1] + zi];
	bab = doubledPermutation[doubledPermutation[doubledPermutation[xi+1] + yi] + zi+1];
	bbb = doubledPermutation[doubledPermutation[doubledPermutation[xi+1] + zi+1] + zi+1];

	double x1, x2, y1, y2;

	// The gradient function calculates the dot product between a pseudorandom 
	// gradient vector and the vector from the input coordinate to the 8
	// surrounding points in its unit cube.
	x1 = lerp(grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf),	u); 
	
	// This is all then (linear extraploted)lerped together as a sort of weighted average based on the faded (u,v,w)
	// values we made earlier.
	x2 = lerp(grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf), u);

	y1 = lerp(x1, x2, v);

	x1 = lerp(grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1), u);
	x2 = lerp(grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1), u);
	y2 = lerp(x1, x2, v);

	// result will range between -1 and 1 //
	double result = lerp(y1, y2, w);

	// For convenience we bound it to 0 - 1 (theoretical min/max before is -1 - 1)
	return (lerp(y1, y2, w) + 1) / 2;						
}

/*
int Perlin::inc(int num) 
{
	num++;
	if (repeat > 0) num %= repeat;

	return num;
}
*/

double Perlin::grad(int hash, double x, double y, double z) {
	int h = hash & 15;									// Take the hashed value and take the first 4 bits of it (15 == 0b1111)
	double u = h < 8 /* 0b1000 */ ? x : y;				// If the most significant bit (MSB) of the hash is 0 then set u = x.  Otherwise y.

	double v;											// In Ken Perlin's original implementation this was another conditional operator (?:).  I
														// expanded it for readability.

	if (h < 4 /* 0b0100 */)								// If the first and second significant bits are 0 set v = y
		v = y;
	else if (h == 12 /* 0b1100 */ || h == 14 /* 0b1110*/)// If the first and second significant bits are 1 set v = x
		v = x;
	else 												// If the first and second significant bits are not equal (0/1, 1/0) set v = z
		v = z;

	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v); // Use the last 2 bits to decide if u and v are positive or negative.  Then return their addition.
}

double Perlin::fade(double t) 
{
	// Fade function as defined by Ken Perlin.  This eases coordinate values
	// so that they will "ease" towards integral values.  This ends up smoothing
	// the final output.
	return t * t * t * (t * (t * 6 - 15) + 10);			// 6t^5 - 15t^4 + 10t^3
}

double Perlin::lerp(double a, double b, double x) {
	return a + x * (b - a);
}


double Perlin::signedOctave(double x, double y, double z)
{
	double total = 0;
	double tmpFrequency = frequency;
	double amplitude = 1.0f;
	double maxValue = 0;			// Used for normalizing result to 0.0 - 1.0

	for (int i = 0; i < octaves; i++) {
		total += generateNoise(x * tmpFrequency, y * tmpFrequency, z * tmpFrequency) * amplitude;
		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2;
	}

	return total / maxValue;
}

double Perlin::unsignedOctave(double x, double y, double z)
{
	double t = signedOctave(x, y, z);
	t = t / 2 + 0.5f;
	return t;
}