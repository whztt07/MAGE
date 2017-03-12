#pragma once

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "math\math.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Declarations and Definitions
//-----------------------------------------------------------------------------
namespace mage {

	/**
	 A struct of transforms.
	*/
	struct SpriteTransform final {

		//-------------------------------------------------------------------------
		// Construction
		//-------------------------------------------------------------------------

		/**
		 Constructs a sprite transform from the given translation, rotation, rotation origin and scale component.

		 @param[in]		translation
						A reference to the translation component.
		 @param[in]		rotation
						A reference to the rotation component.
		 @param[in]		rotation_origin
						A reference to the rotation component.
		 @param[in]		scale
						A reference to the scale component.
		 */
		SpriteTransform(const XMFLOAT2 &translation = { 0.0f, 0.0f }, const XMFLOAT2 &rotation = { 0.0f, 0.0f },
			const XMFLOAT2 &rotation_origin = { 0.0f, 0.0f }, const XMFLOAT2 &scale = { 1.0f, 1.0f })
			: m_translation(translation), m_rotation(rotation), m_rotation_origin(rotation_origin), m_scale(scale) {}

		/**
		 Constructs a sprite transform from the given sprite transform.

		 @param[in]		transform
						The sprite transform.
		 */
		SpriteTransform(const SpriteTransform &transform) = default;

		//-------------------------------------------------------------------------
		// Destruction
		//-------------------------------------------------------------------------

		/**
		 Destructs this sprite transform.
		 */
		~SpriteTransform() = default;

		//-------------------------------------------------------------------------
		// Copying
		//-------------------------------------------------------------------------

		/**
		 Copies the given sprite transform to this sprite transform.

		 @param[in]		transform
						The sprite transform to copy from.
		 @return		A reference to the copy of the given sprite transform
						(i.e. this sprite transform).
		 */
		SpriteTransform &operator=(const SpriteTransform &transform) = default;

		//-------------------------------------------------------------------------
		// Tranbslation + Rotation + Scale
		//-------------------------------------------------------------------------

		/**
		 Sets the translation, rotation, scale component of this sprite transform
		 to the given components.

		 @param[in]		translation
						A reference to the translation component.
		 @param[in]		rotation
						A reference to the rotation component.
		 @param[in]		rotation_origin
						A reference to the rotation component.
		 @param[in]		scale
						A reference to the scale component.
		 */
		void SetComponents(const XMFLOAT2 &translation, const XMFLOAT2 &rotation, const XMFLOAT2 &rotation_origin, const XMFLOAT2 &scale) {
			m_translation = translation;
			m_rotation = rotation;
			m_rotation_origin = rotation_origin;
			m_scale = scale;
		}

		//-------------------------------------------------------------------------
		// Translation
		//-------------------------------------------------------------------------

		/**
		 Sets the x-value of the translation component of this sprite transform to the given value.

		 @param[in]		x
						The x-value of the translation component.
		 */
		void SetTranslationX(float x) {
			m_translation.x = x;
		}

		/**
		 Sets the y-value of the translation component of this sprite transform to the given value.

		 @param[in]		y
						The y-value of the translation component.
		 */
		void SetTranslationY(float y) {
			m_translation.y = y;
		}

		/**
		 Sets the translation component of this sprite transform to the given translation component.

		 @param[in]		x
						The x-value of the translation component.
		 @param[in]		y
						The y-value of the translation component.
		 */
		void SetTranslation(float x, float y) {
			m_translation.x = x;
			m_translation.y = y;
		}

		/**
		 Sets the translation component of this sprite transform to the given translation component.

		 @param[in]		translation
						A reference to the translation component.
		 */
		void SetTranslation(const XMFLOAT2 &translation) {
			m_translation = translation;
		}

		/**
		 Adds the given x-value to the translation component of this sprite transform.

		 @param[in]		x
						The x-value of the translation component to add.
		 */
		void AddTranslationX(float x) {
			m_translation.x += x;
		}

		/**
		 Adds the given y-value to the translation component of this sprite transform.

		 @param[in]		y
						The y-value of the translation component to add.
		 */
		void AddTranslationY(float y) {
			m_translation.y += y;
		}

		/**
		 Adds the given translation component to the translation component of this sprite transform.

		 @param[in]		x
						The x-value of the translation component to add.
		 @param[in]		y
						The y-value of the translation component to add.
		 */
		void AddTranslation(float x, float y) {
			m_translation.x += x;
			m_translation.y += y;
		}

		/**
		 Adds the given translation component to the translation component of this sprite transform.

		 @param[in]		translation
						A reference to the translation component to add.
		 */
		void AddTranslation(const XMFLOAT2 &translation) {
			AddTranslation(translation.x, translation.y);
		}

		/**
		 Returns the x-value of the translation component of this sprite transform.

		 @return		The x-value of the translation component of this sprite transform.
		 */
		float GetTranslationX() const {
			return m_translation.x;
		}

		/**
		 Returns the y-value of the translation component of this sprite transform.

		 @return		The y-value of the translation component of this sprite transform.
		 */
		float GetTranslationY() const {
			return m_translation.y;
		}

		/**
		 Returns the translation component of this sprite transform.

		 @return		The translation component of this sprite transform.
		 */
		XMFLOAT2 GetTranslation() const {
			return m_translation;
		}

		//-------------------------------------------------------------------------
		// Rotation
		//-------------------------------------------------------------------------

		/**
		 Sets the x-value of the rotation component of this sprite transform to the given value.

		 @param[in]		x
						The x-value of the rotation component.
		 */
		void SetRotationX(float x) {
			m_rotation.x = x;
		}

		/**
		 Sets the y-value of the rotation component of this sprite transform to the given value.

		 @param[in]		y
						The y-value of the rotation component.
		 */
		void SetRotationY(float y) {
			m_rotation.y = y;
		}

		/**
		 Sets the rotation component of this sprite transform to the given rotation component.

		 @param[in]		x
						The x-value of the rotation component.
		 @param[in]		y
						The y-value of the rotation component.
		 */
		void SetRotation(float x, float y) {
			m_rotation.x = x;
			m_rotation.y = y;
		}

		/**
		 Sets the rotation component of this sprite transform to the given rotation component.

		 @param[in]		rotation
						A reference to the rotation component.
		 */
		void SetRotation(const XMFLOAT2 &rotation) {
			m_rotation = rotation;
		}

		/**
		 Adds the given x-value to the rotation component of this sprite transform.

		 @param[in]		x
						The x-value of the rotation component to add.
		 */
		void AddRotationX(float x) {
			m_rotation.x += x;
		}

		/**
		 Adds the given y-value to the rotation component of this sprite transform.

		 @param[in]		y
						The y-value of the rotation component to add.
		 */
		void AddRotationY(float y) {
			m_rotation.y += y;
		}

		/**
		 Adds the given rotation component to the rotation component of this sprite transform.

		 @param[in]		x
						The x-value of the rotation component to add.
		 @param[in]		y
						The y-value of the rotation component to add.
		 */
		void AddRotation(float x, float y) {
			m_rotation.x += x;
			m_rotation.y += y;
		}

		/**
		 Adds the given rotation component to the rotation component of this sprite transform.

		 @param[in]		rotation
						A reference to the rotation component to add.
		 */
		void AddRotation(const XMFLOAT2 &rotation) {
			AddRotation(rotation.x, rotation.y);
		}

		/**
		 Returns the x-value of the rotation component of this sprite transform.

		 @return		The x-value of the rotation component of this sprite transform.
		 */
		float GetRotationX() const {
			return m_rotation.x;
		}

		/**
		 Returns the y-value of the rotation component of this sprite transform.

		 @return		The y-value of the rotation component of this sprite transform.
		 */
		float GetRotationY() const {
			return m_rotation.y;
		}

		/**
		 Returns the rotation component of this sprite transform.

		 @return		The rotation component of this sprite transform.
		 */
		XMFLOAT2 GetRotation() const {
			return m_rotation;
		}

		//-------------------------------------------------------------------------
		// Rotation
		//-------------------------------------------------------------------------

		/**
		 Sets the x-value of the rotation origin of this sprite transform to the given value.

		 @param[in]		x
						The x-value of the rotation origin.
		 */
		void SetRotationOriginX(float x) {
			m_rotation_origin.x = x;
		}

		/**
		 Sets the y-value of the rotation origin of this sprite transform to the given value.

		 @param[in]		y
						The y-value of the rotation origin.
		 */
		void SetRotationOriginY(float y) {
			m_rotation_origin.y = y;
		}

		/**
		 Sets the rotation origin of this sprite transform to the given rotation origin.

		 @param[in]		x
						The x-value of the rotation origin.
		 @param[in]		y
						The y-value of the rotation origin.
		 */
		void SetRotationOrigin(float x, float y) {
			m_rotation_origin.x = x;
			m_rotation_origin.y = y;
		}

		/**
		 Sets the rotation origin of this sprite transform to the given rotation origin.

		 @param[in]		rotation_origin
						A reference to the rotation origin.
		 */
		void SetRotationOrigin(const XMFLOAT2 &rotation_origin) {
			m_rotation_origin = rotation_origin;
		}

		/**
		 Adds the given x-value to the rotation origin of this sprite transform.

		 @param[in]		x
						The x-value of the rotation origin to add.
		 */
		void AddRotationOriginX(float x) {
			m_rotation_origin.x += x;
		}

		/**
		 Adds the given y-value to the rotation origin of this sprite transform.

		 @param[in]		y
						The y-value of the rotation origin to add.
		 */
		void AddRotationOriginY(float y) {
			m_rotation_origin.y += y;
		}

		/**
		 Adds the given rotation origin to the rotation origin of this sprite transform.

		 @param[in]		x
						The x-value of the rotation origin to add.
		 @param[in]		y
						The y-value of the rotation origin to add.
		 */
		void AddRotationOrigin(float x, float y) {
			m_rotation_origin.x += x;
			m_rotation_origin.y += y;
		}

		/**
		 Adds the given rotation origin to the rotation origin of this sprite transform.

		 @param[in]		rotation_origin
						A reference to the rotation origin to add.
		 */
		void AddRotationOrigin(const XMFLOAT2 &rotation_origin) {
			AddRotationOrigin(rotation_origin.x, rotation_origin.y);
		}

		/**
		 Returns the x-value of the rotation origin of this sprite transform.

		 @return		The x-value of the rotation origin of this sprite transform.
		 */
		float GetRotationOriginX() const {
			return m_rotation_origin.x;
		}

		/**
		 Returns the y-value of the rotation origin of this sprite transform.

		 @return		The y-value of the rotation origin of this sprite transform.
		 */
		float GetRotationOriginY() const {
			return m_rotation_origin.y;
		}

		/**
		 Returns the rotation origin of this sprite transform.

		 @return		The rotation origin of this sprite transform.
		 */
		XMFLOAT2 GetRotationOrigin() const {
			return m_rotation_origin;
		}

		//-------------------------------------------------------------------------
		// Scale
		//-------------------------------------------------------------------------

		/**
		 Sets the x-value of the scale component of this sprite transform to the given value.

		 @param[in]		x
						The x-value of the scale component.
		 */
		void SetScaleX(float x) {
			m_scale.x = x;
		}

		/**
		 Sets the y-value of the scale component of this sprite transform to the given value.

		 @param[in]		y
						The y-value of the scale component.
		 */
		void SetScaleY(float y) {
			m_scale.y = y;
		}

		/**
		 Sets the scale component of this sprite transform to the given scale component.

		 @param[in]		x
						The x-value of the scale component.
		 @param[in]		y
						The y-value of the scale component.
		 */
		void SetScale(float x, float y) {
			m_scale.x = x;
			m_scale.y = y;
		}

		/**
		 Sets the scale component of this sprite transform to the given scale component.

		 @param[in]		scale
						A reference to the scale component.
		 */
		void SetScale(const XMFLOAT2 &scale) {
			m_scale = scale;
		}

		/**
		 Adds the given x-value to the scale component of this sprite transform.

		 @param[in]		x
						The x-value of the scale component to add.
		 */
		void AddScaleX(float x) {
			m_scale.x += x;
		}

		/**
		 Adds the given y-value to the scale component of this sprite transform.

		 @param[in]		y
						The y-value of the scale component to add.
		 */
		void AddScaleY(float y) {
			m_scale.y += y;
		}

		/**
		 Adds the given scale component to the scale component of this sprite transform.

		 @param[in]		x
						The x-value of the scale component to add.
		 @param[in]		y
						The y-value of the scale component to add.
		 */
		void AddScale(float x, float y) {
			m_scale.x += x;
			m_scale.y += y;
		}

		/**
		 Adds the given scale component to the scale component of this sprite transform.

		 @param[in]		scale
						A reference to the scale component to add.
		 */
		void AddScale(const XMFLOAT2 &scale) {
			AddScale(scale.x, scale.y);
		}

		/**
		 Returns the x-value of the scale component of this sprite transform.

		 @return		The x-value of the scale component of this sprite transform.
		 */
		float GetScaleX() const {
			return m_scale.x;
		}

		/**
		 Returns the y-value of the scale component of this sprite transform.

		 @return		The y-value of the scale component of this sprite transform.
		 */
		float GetScaleY() const {
			return m_scale.y;
		}

		/**
		 Returns the scale component of this sprite transform.

		 @return		The scale component of this sprite transform.
		 */
		XMFLOAT2 GetScale() const {
			return m_scale;
		}

	private:

		/**
		 The translation component of this sprite transform.
		 */
		XMFLOAT2 m_translation;

		/**
		 The rotation component (in radians) of this sprite transform.
		 */
		XMFLOAT2 m_rotation;

		/**
		 The rotation origin of this sprite transform.
		 */
		XMFLOAT2 m_rotation_origin;

		/**
		 The scale component of this sprite transform.
		 */
		XMFLOAT2 m_scale;
	};
}