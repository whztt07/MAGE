#ifndef MAGE_HEADER_BASIS
#define MAGE_HEADER_BASIS

//-----------------------------------------------------------------------------
// Engine Declarations and Definitions
//-----------------------------------------------------------------------------

/**
 Calculates an orthonormal basis from a given unit vector with the method of 
 Hughes and M�ller.

 @pre			@a n is normalized.
 @param[in]		n
				A basis vector of the orthonormal basis.
 @return		An orthonormal basis.
 */
float3x3 OrthonormalBasis_HughesMoller(float3 n) {
	const float3 n_ortho = (0.1f < abs(n.x)) ? float3(0.0f, 1.0f, 0.0f) 
		                                     : float3(1.0f, 0.0f, 0.0f);
	const float3 t = normalize(cross(n_ortho, n));
	const float3 b = cross(n, t);

	return float3x3(t, b, n);
}

/**
 Calculates an orthonormal basis from a given unit vector with the method of 
 Duff, Burgess, Christensen, Hery, Kensler, Liani and Villemin.

 @pre			@a n is normalized.
 @param[in]		n
				A basis vector of the orthonormal basis.
 @return		An orthonormal basis.
 */
float3x3 OrthonormalBasis_Duff(float3 n) {
	const float  s  = (0.0f > n.z) ? -1.0f : 1.0f;
	const float  a0 = -1.0f / (s + n.z);
	const float  a1 = n.x * n.y * a0;

	const float3 t  = float3(1.0f + s * n.x * n.x * a0, s * a1, -s * n.x);
	const float3 b  = float3(a1, s + n.y * n.y * a0, -n.y);

	return float3x3(t, b, n);
}

/**
 Calculates an orthonormal basis from a given unit vector with the method.

 @pre			@a n is normalized.
 @param[in]		n
				A basis vector of the orthonormal basis.
 @return		An orthonormal basis.
 */
float3x3 OrthonormalBasis(float3 n) {
	return OrthonormalBasis_Duff(n);
}

#endif //MAGE_HEADER_BASIS